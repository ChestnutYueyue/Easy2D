#include <easy2d/e2dbase.h>
#include <easy2d/e2dcommon.h>
#include <easy2d/e2dtool.h>
#include <unordered_map>
#include <vector>

// 定义 STB_IMAGE_IMPLEMENTATION 以创建实现
#define STB_IMAGE_IMPLEMENTATION
#include <easy2d/stb_image.h>

namespace {
std::unordered_map<easy2d::String, easy2d::Image *> s_mImagesFromFile;
std::unordered_map<easy2d::Resource, easy2d::Image *> s_mImagesFromResource;
} // namespace

namespace easy2d {
HRESULT LoadBitmapFromFile(ID2D1Bitmap **ppBitmap, const String &filePath);
HRESULT LoadBitmapFromResource(ID2D1Bitmap **ppBitmap, const Resource &res);
HRESULT CreateBitmapFromMemory(ID2D1Bitmap **ppBitmap, unsigned char *data,
                               int width, int height, int channels);
} // namespace easy2d

easy2d::Image::Image(ID2D1Bitmap *bitmap) : _bitmap(bitmap) {
  if (_bitmap) {
    _bitmap->AddRef();
  }
}

easy2d::Image::~Image() { SafeRelease(_bitmap); }

ID2D1Bitmap *easy2d::Image::getBitmap() { return _bitmap; }

void easy2d::Image::resetBitmap(ID2D1Bitmap *bitmap) {
  SafeRelease(_bitmap);
  _bitmap = bitmap;
  if (_bitmap) {
    _bitmap->AddRef();
  }
}

float easy2d::Image::getWidth() const {
  if (_bitmap) {
    return _bitmap->GetSize().width;
  } else {
    return 0;
  }
}

float easy2d::Image::getHeight() const {
  if (_bitmap) {
    return _bitmap->GetSize().height;
  } else {
    return 0;
  }
}

easy2d::Size easy2d::Image::getSize() const {
  if (_bitmap) {
    return Size(getWidth(), getHeight());
  } else {
    return Size();
  }
}

easy2d::Image *easy2d::Image::load(const String &filePath) {
  auto iter = s_mImagesFromFile.find(filePath);
  if (iter != s_mImagesFromFile.end()) {
    return iter->second;
  }

  ID2D1Bitmap *pBitmap = nullptr;
  HRESULT hr = LoadBitmapFromFile(&pBitmap, filePath);

  Image *image = nullptr;
  if (SUCCEEDED(hr)) {
    image = gcnew Image(pBitmap);
    image->retain();
    s_mImagesFromFile.insert(std::make_pair(filePath, image));
  }

  SafeRelease(pBitmap);
  if (SUCCEEDED(hr)) {
    return image;
  }

  E2D_ERROR("Load image failed! ERROR_CODE = %#X", hr);
  return nullptr;
}

easy2d::Image *easy2d::Image::load(const Resource &res) {
  auto iter = s_mImagesFromResource.find(res);
  if (iter != s_mImagesFromResource.end()) {
    return iter->second;
  }

  ID2D1Bitmap *pBitmap = nullptr;
  HRESULT hr = LoadBitmapFromResource(&pBitmap, res);

  Image *image = nullptr;
  if (SUCCEEDED(hr)) {
    image = gcnew Image(pBitmap);
    image->retain();
    s_mImagesFromResource.insert(std::make_pair(res, image));
  }

  SafeRelease(pBitmap);
  if (SUCCEEDED(hr)) {
    return image;
  }

  E2D_ERROR("Load image failed! ERROR_CODE = %#X", hr);
  return nullptr;
}

easy2d::Image *easy2d::Image::load(int resNameId, const String &resType) {
  return load(Resource{resNameId, resType});
}

namespace {}

void easy2d::Image::clearCache() {
  for (auto pair : s_mImagesFromFile) {
    GC::release(pair.second);
  }
  s_mImagesFromFile.clear();

  for (auto pair : s_mImagesFromResource) {
    GC::release(pair.second);
  }
  s_mImagesFromResource.clear();
}

void easy2d::Image::reloadCache() {
  for (auto pair : s_mImagesFromFile) {
    ID2D1Bitmap *pBitmap = nullptr;
    HRESULT hr = LoadBitmapFromFile(&pBitmap, pair.first);

    if (SUCCEEDED(hr)) {
      pair.second->resetBitmap(pBitmap);
    } else {
      E2D_ERROR("Reload image failed! ERROR_CODE = %#X", hr);
    }
    SafeRelease(pBitmap);
  }
  for (auto pair : s_mImagesFromResource) {
    ID2D1Bitmap *pBitmap = nullptr;
    HRESULT hr = LoadBitmapFromResource(&pBitmap, pair.first);

    if (SUCCEEDED(hr)) {
      pair.second->resetBitmap(pBitmap);
    } else {
      E2D_ERROR("Reload image failed! ERROR_CODE = %#X", hr);
    }
    SafeRelease(pBitmap);
  }
}

namespace easy2d {

/**
 * @brief 从内存数据创建 ID2D1Bitmap
 * @param ppBitmap 输出位图指针
 * @param data 图像像素数据
 * @param width 图像宽度
 * @param height 图像高度
 * @param channels 通道数 (3=RGB, 4=RGBA)
 * @return HRESULT 操作结果
 */
HRESULT CreateBitmapFromMemory(ID2D1Bitmap **ppBitmap, unsigned char *data,
                               int width, int height, int channels) {
  if (!data || !ppBitmap || width <= 0 || height <= 0) {
    return E_INVALIDARG;
  }

  ID2D1RenderTarget *pRenderTarget = Renderer::getRenderTarget();
  if (!pRenderTarget) {
    return E_FAIL;
  }

  // 设置位图属性
  D2D1_BITMAP_PROPERTIES bitmapProperties = {};
  bitmapProperties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
  bitmapProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
  bitmapProperties.dpiX = 96.0f;
  bitmapProperties.dpiY = 96.0f;

  // stb_image 加载的数据是 RGB/RGBA 格式，需要转换为 BGRA 格式
  // 同时需要处理预乘 alpha
  std::vector<unsigned char> convertedData(width * height * 4);

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int srcIdx = (y * width + x) * channels;
      int dstIdx = (y * width + x) * 4; // 保持原方向，不进行垂直翻转

      unsigned char r = data[srcIdx];
      unsigned char g = (channels >= 2) ? data[srcIdx + 1] : r;
      unsigned char b = (channels >= 3) ? data[srcIdx + 2] : r;
      unsigned char a = (channels >= 4) ? data[srcIdx + 3] : 255;

      // 预乘 alpha
      float alpha = a / 255.0f;
      convertedData[dstIdx] = static_cast<unsigned char>(b * alpha);     // B
      convertedData[dstIdx + 1] = static_cast<unsigned char>(g * alpha); // G
      convertedData[dstIdx + 2] = static_cast<unsigned char>(r * alpha); // R
      convertedData[dstIdx + 3] = a;                                     // A
    }
  }

  D2D1_SIZE_U size =
      D2D1::SizeU(static_cast<UINT32>(width), static_cast<UINT32>(height));
  HRESULT hr = pRenderTarget->CreateBitmap(
      size, convertedData.data(), width * 4, &bitmapProperties, ppBitmap);

  return hr;
}

HRESULT LoadBitmapFromFile(ID2D1Bitmap **ppBitmap,
                           const easy2d::String &filePath) {
  String actualFilePath = Path::searchForFile(filePath);
  if (actualFilePath.empty()) {
    return S_FALSE;
  }

  // 使用 stb_image 加载图片
  int width, height, channels;
  unsigned char *data =
      stbi_load(actualFilePath.c_str(), &width, &height, &channels, 0);

  if (!data) {
    E2D_ERROR("stbi_load failed: %s", stbi_failure_reason());
    return E_FAIL;
  }

  HRESULT hr = CreateBitmapFromMemory(ppBitmap, data, width, height, channels);

  // 释放 stb_image 分配的内存
  stbi_image_free(data);

  return hr;
}

HRESULT LoadBitmapFromResource(ID2D1Bitmap **ppBitmap,
                               const easy2d::Resource &res) {
  auto resData = res.loadData();
  if (!resData.isValid()) {
    return E_FAIL;
  }

  // 使用 stb_image 从内存加载图片
  int width, height, channels;
  unsigned char *data =
      stbi_load_from_memory(reinterpret_cast<unsigned char *>(resData.buffer),
                            resData.size, &width, &height, &channels, 0);

  if (!data) {
    E2D_ERROR("stbi_load_from_memory failed: %s", stbi_failure_reason());
    return E_FAIL;
  }

  HRESULT hr = CreateBitmapFromMemory(ppBitmap, data, width, height, channels);

  // 释放 stb_image 分配的内存
  stbi_image_free(data);

  return hr;
}

} // namespace easy2d
