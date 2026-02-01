#define STB_IMAGE_IMPLEMENTATION
#include <easy2d/GLTexture.h>
#include <easy2d/e2dbase.h>
#include <stb_image.h>
#include <fstream>
#include <vector>

namespace easy2d
{

// GLTexture 实现

GLTexture::GLTexture()
    : _textureID(0)
    , _width(0)
    , _height(0)
    , _format(GL_RGBA)
{
}

GLTexture::~GLTexture()
{
    release();
}

void GLTexture::release()
{
    if (_textureID != 0)
    {
        glDeleteTextures(1, &_textureID);
        _textureID = 0;
    }
    _width = 0;
    _height = 0;
}

bool GLTexture::loadFromFile(const std::string& filePath)
{
    // 读取文件数据
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        E2D_ERROR("Failed to open texture file: %s", filePath.c_str());
        return false;
    }

    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> buffer(fileSize);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    file.close();

    return loadFromMemory(buffer.data(), fileSize);
}

bool GLTexture::loadFromMemory(const unsigned char* data, size_t size)
{
    // 使用stb_image加载图片
    int channels;
    unsigned char* pixels = stbi_load_from_memory(data, static_cast<int>(size), &_width, &_height, &channels, STBI_rgb_alpha);
    
    if (!pixels)
    {
        E2D_ERROR("Failed to load image from memory: %s", stbi_failure_reason());
        return false;
    }

    bool result = createFromRawData(_width, _height, pixels, true);
    stbi_image_free(pixels);

    return result;
}

bool GLTexture::createFromRawData(int width, int height, const unsigned char* data, bool generateMipmap)
{
	release();

	_width = width;
	_height = height;
	_format = GL_RGBA;

	// 生成纹理
	glGenTextures(1, &_textureID);
	glBindTexture(GL_TEXTURE_2D, _textureID);

	// 设置纹理参数
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, generateMipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// 上传纹理数据
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	// 生成mipmap
	if (generateMipmap)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

bool GLTexture::createFromRawDataForGlyph(int width, int height, const unsigned char* data)
{
	release();

	_width = width;
	_height = height;
	_format = GL_RGBA;

	// 生成纹理
	glGenTextures(1, &_textureID);
	glBindTexture(GL_TEXTURE_2D, _textureID);

	// 设置纹理参数 - 使用NEAREST过滤保持边缘锐利
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// 上传纹理数据
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

bool GLTexture::createEmpty(int width, int height, GLenum format)
{
    release();

    _width = width;
    _height = height;
    _format = format;

    // 生成纹理
    glGenTextures(1, &_textureID);
    glBindTexture(GL_TEXTURE_2D, _textureID);

    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 创建空纹理
    GLenum internalFormat = (format == GL_RGB) ? GL_RGB : GL_RGBA;
    GLenum type = GL_UNSIGNED_BYTE;
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, _width, _height, 0, format, type, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

void GLTexture::bind(unsigned int unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, _textureID);
}

void GLTexture::unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLTexture::setFilterMode(GLenum minFilter, GLenum magFilter)
{
    glBindTexture(GL_TEXTURE_2D, _textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLTexture::setWrapMode(GLenum wrapS, GLenum wrapT)
{
    glBindTexture(GL_TEXTURE_2D, _textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLTexture::generateMipmap()
{
    glBindTexture(GL_TEXTURE_2D, _textureID);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// GLTextureCache 实现

GLTextureCache& GLTextureCache::getInstance()
{
    static GLTextureCache instance;
    return instance;
}

GLTextureCache::~GLTextureCache()
{
    clearCache();
}

GLTexture* GLTextureCache::getTexture(const std::string& filePath)
{
    // 查找缓存
    auto it = _textureMap.find(filePath);
    if (it != _textureMap.end())
    {
        return it->second;
    }

    // 加载新纹理
    GLTexture* texture = new GLTexture();
    if (!texture->loadFromFile(filePath))
    {
        delete texture;
        return nullptr;
    }

    _textureMap[filePath] = texture;
    return texture;
}

GLTexture* GLTextureCache::getTexture(const Resource& res)
{
    // 使用资源ID作为缓存键
    std::string key = std::to_string(res.getId()) + ":" + res.getType();
    
    // 查找缓存
    auto it = _textureMap.find(key);
    if (it != _textureMap.end())
    {
        return it->second;
    }

    // 加载资源数据
    Resource::Data data = res.loadData();
    if (!data.isValid())
    {
        E2D_ERROR("Failed to load resource data for texture");
        return nullptr;
    }

    // 加载纹理
    GLTexture* texture = new GLTexture();
    if (!texture->loadFromMemory(static_cast<const unsigned char*>(data.buffer), data.size))
    {
        delete texture;
        return nullptr;
    }

    _textureMap[key] = texture;
    return texture;
}

void GLTextureCache::addTexture(const std::string& key, GLTexture* texture)
{
    // 如果已存在，先删除旧的
    auto it = _textureMap.find(key);
    if (it != _textureMap.end())
    {
        delete it->second;
    }
    _textureMap[key] = texture;
}

void GLTextureCache::removeTexture(const std::string& key)
{
    auto it = _textureMap.find(key);
    if (it != _textureMap.end())
    {
        delete it->second;
        _textureMap.erase(it);
    }
}

void GLTextureCache::clearCache()
{
    for (auto& pair : _textureMap)
    {
        delete pair.second;
    }
    _textureMap.clear();
}

void GLTextureCache::reloadAll()
{
    // 注意：OpenGL纹理在上下文丢失后需要重新创建
    // 这里只是清空缓存，下次访问时会重新加载
    clearCache();
}

} // namespace easy2d
