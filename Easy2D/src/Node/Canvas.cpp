#include <easy2d/node/e2dcanvas.h>

easy2d::Canvas::Canvas(const Size &size)
    : _rt(nullptr), _state(nullptr), _outputImage(nullptr), _size(size) {}

easy2d::Canvas::~Canvas() {
  SafeRelease(_state);
  SafeRelease(_rt);
  GC::release(_outputImage);
}

easy2d::Image *easy2d::Canvas::getOutputImage() const { return _outputImage; }

easy2d::CanvasBrush *easy2d::Canvas::beginDraw() {
  HRESULT hr = S_OK;
  if (!_rt) {
    ID2D1BitmapRenderTarget *rt = nullptr;
    hr = Renderer::getRenderTarget()->CreateCompatibleRenderTarget(
        reinterpret_cast<const D2D1_SIZE_F &>(_size), &rt);

    if (SUCCEEDED(hr)) {
      ID2D1Bitmap *bitmap = nullptr;
      hr = rt->GetBitmap(&bitmap);

      if (SUCCEEDED(hr)) {
        _rt = rt;
        _rt->AddRef();

        if (_outputImage) {
          _outputImage->resetBitmap(bitmap);
        } else {
          _outputImage = gcnew Image(bitmap);
          GC::retain(_outputImage);
        }
      }
      SafeRelease(bitmap);
    }
    SafeRelease(rt);

    if (FAILED(hr)) {
      E2D_ERROR("Canvas beginDraw failed! ERR_CODE=%#X", hr);
    }
  }

  if (SUCCEEDED(hr)) {
    hr = Renderer::getID2D1Factory()->CreateDrawingStateBlock(&_state);
  }

  if (FAILED(hr)) {
    E2D_ERROR("Canvas create drawing state block failed! ERR_CODE=%#X", hr);
    return nullptr;
  }

  _rt->SaveDrawingState(_state);
  _rt->BeginDraw();
  return gcnew CanvasBrush(this);
}

void easy2d::Canvas::endDraw() {
  _rt->EndDraw();

  if (_state) {
    _rt->RestoreDrawingState(_state);
  }
}

//
// CanvasBrush
//

easy2d::CanvasBrush::CanvasBrush(easy2d::Canvas *canvas)
    : _dirtyTransform(false), _rt(canvas->_rt), _brush(nullptr),
      _interpolationMode(InterpolationMode::Linear), _opacity(1.0f),
      _rotation(0.f), _pos(), _scale(1.f, 1.f), _skew(), _style() {
  _rt->AddRef();

  HRESULT hr =
      _rt->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &_brush);

  if (FAILED(hr)) {
    E2D_ERROR("Canvas CreateSolidColorBrush failed! ERR_CODE=%#X", hr);
  }
}

easy2d::CanvasBrush::~CanvasBrush() {
  SafeRelease(_brush);
  SafeRelease(_rt);
}

void easy2d::CanvasBrush::_updateTransform() {
  if (_dirtyTransform) {
    Matrix33 transform = Matrix33::scaling(_scale.x, _scale.y) *
                         Matrix33::skewing(_skew.x, _skew.y) *
                         Matrix33::rotation(_rotation) *
                         Matrix33::translation(_pos.x, _pos.y);
    _rt->SetTransform(transform.toD2DMatrix());
    _dirtyTransform = false;
  }
}

void easy2d::CanvasBrush::drawShape(Shape *shape) {
  _updateTransform();
  _brush->SetOpacity(_opacity);
  if (_style.mode == DrawingStyle::Mode::Fill ||
      _style.mode == DrawingStyle::Mode::Round) {
    ID2D1StrokeStyle *strokeStyle = nullptr;
    switch (_style.lineJoin) {
    case LineJoin::Miter:
      strokeStyle = Renderer::getMiterID2D1StrokeStyle();
      break;
    case LineJoin::Bevel:
      strokeStyle = Renderer::getBevelID2D1StrokeStyle();
      break;
    case LineJoin::Round:
      strokeStyle = Renderer::getRoundID2D1StrokeStyle();
      break;
    case LineJoin::None:
      break;
    }
    _brush->SetColor(
        reinterpret_cast<const D2D1_COLOR_F &>(_style.strokeColor));
    _rt->DrawGeometry(shape->_geo, _brush, _style.strokeWidth, strokeStyle);
  }

  if (_style.mode == DrawingStyle::Mode::Fill ||
      _style.mode == DrawingStyle::Mode::Solid) {
    _brush->SetColor(reinterpret_cast<const D2D1_COLOR_F &>(_style.fillColor));
    _rt->FillGeometry(shape->_geo, _brush);
  }
}

void easy2d::CanvasBrush::drawImage(Image *image, const Point &pos,
                                    const Rect &cropRect) {
  drawImage(image, Rect(pos, image->getSize()), cropRect);
}

void easy2d::CanvasBrush::drawImage(Image *image, const Rect &destRect,
                                    const Rect &cropRect) {
  _updateTransform();
  const Rect *srcRect = nullptr;
  if (!cropRect.isEmpty()) {
    srcRect = &cropRect;
  }

  _rt->DrawBitmap(image->getBitmap(),
                  reinterpret_cast<const D2D1_RECT_F &>(destRect), _opacity,
                  (_interpolationMode == InterpolationMode::Nearest)
                      ? D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
                      : D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
                  reinterpret_cast<const D2D1_RECT_F *>(srcRect));
}

void easy2d::CanvasBrush::drawImage(KeyFrame *frame, const Point &pos) {
  auto image = frame->getImage();
  drawImage(image, Rect(pos, image->getSize()), frame->getCropRect());
}

void easy2d::CanvasBrush::drawImage(KeyFrame *frame, const Rect &destRect) {
  auto image = frame->getImage();
  drawImage(image, destRect, frame->getCropRect());
}

void easy2d::CanvasBrush::drawText(TextLayout *layout, const Point &pos) {
  _updateTransform();
  _brush->SetOpacity(_opacity);
  Renderer::DrawTextLayout(layout, _style, pos, _rt, _brush);
}

void easy2d::CanvasBrush::drawText(const String &text, const Point &pos,
                                   const TextStyle &style) {
  TextLayout layout(text, style);
  drawText(&layout, pos);
}

void easy2d::CanvasBrush::clear() { _rt->Clear(); }

void easy2d::CanvasBrush::clear(const Color &color) {
  _rt->Clear(reinterpret_cast<const D2D1_COLOR_F &>(color));
}

easy2d::Color easy2d::CanvasBrush::getFillColor() const {
  return _style.fillColor;
}

easy2d::Color easy2d::CanvasBrush::getStrokeColor() const {
  return _style.strokeColor;
}

float easy2d::CanvasBrush::getStrokeWidth() const { return _style.strokeWidth; }

void easy2d::CanvasBrush::setFillColor(Color fillColor) {
  _style.fillColor = fillColor;
}

void easy2d::CanvasBrush::setStrokeColor(Color strokeColor) {
  _style.strokeColor = strokeColor;
}

void easy2d::CanvasBrush::setStrokeWidth(float strokeWidth) {
  _style.strokeWidth = strokeWidth;
}

void easy2d::CanvasBrush::setLineJoin(LineJoin lineJoin) {
  _style.lineJoin = lineJoin;
}

easy2d::DrawingStyle::Mode easy2d::CanvasBrush::getDrawingMode() const {
  return _style.mode;
}

void easy2d::CanvasBrush::setDrawingMode(DrawingStyle::Mode mode) {
  _style.mode = mode;
}

easy2d::DrawingStyle easy2d::CanvasBrush::getDrawingStyle() const {
  return _style;
}

void easy2d::CanvasBrush::setDrawingStyle(DrawingStyle style) {
  _style = style;
}

float easy2d::CanvasBrush::getOpacity() const { return _opacity; }

void easy2d::CanvasBrush::setOpacity(float opacity) {
  _opacity = std::min(std::max(opacity, 1.0f), 0.0f);
}

easy2d::Point easy2d::CanvasBrush::getPos() const { return _pos; }

void easy2d::CanvasBrush::setPos(const Point &point) {
  if (_pos != point) {
    _pos = point;
    _dirtyTransform = true;
  }
}

void easy2d::CanvasBrush::movePos(const Vector2 &point) {
  setPos(getPos() + point);
}

float easy2d::CanvasBrush::getRotation() const { return _rotation; }

void easy2d::CanvasBrush::setRotation(float rotation) {
  if (_rotation != rotation) {
    _rotation = rotation;
    _dirtyTransform = true;
  }
}

easy2d::Vector2 easy2d::CanvasBrush::getScale() const { return _scale; }

void easy2d::CanvasBrush::setScale(const Vector2 &scale) {
  if (_scale != scale) {
    _scale = scale;
    _dirtyTransform = true;
  }
}

easy2d::Vector2 easy2d::CanvasBrush::getSkew() const { return _skew; }

void easy2d::CanvasBrush::setSkew(const Vector2 &skew) {
  if (_skew != skew) {
    _skew = skew;
    _dirtyTransform = true;
  }
}

easy2d::Matrix32 easy2d::CanvasBrush::getTransform() const {
  D2D1_MATRIX_3X2_F d2dMatrix;
  _rt->GetTransform(&d2dMatrix);
  // 将D2D1::Matrix3x2F转换为Matrix33（使用6参数构造函数）
  // D2D矩阵: _11 _12 0 | _21 _22 0 | _31 _32 1
  // 对应:     m00 m01 m02 | m10 m11 m12
  Matrix33 transform(d2dMatrix._11, d2dMatrix._12, d2dMatrix._31,
                     d2dMatrix._21, d2dMatrix._22, d2dMatrix._32);
  return transform;
}

easy2d::InterpolationMode easy2d::CanvasBrush::getInterpolationMode() const {
  return _interpolationMode;
}

void easy2d::CanvasBrush::setInterpolationMode(InterpolationMode mode) {
  _interpolationMode = mode;
}
