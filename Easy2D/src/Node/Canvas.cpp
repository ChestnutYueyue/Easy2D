#include <easy2d/e2dcanvas.h>

easy2d::Canvas::Canvas(const Size& size)
	: _outputImage(nullptr)
	, _rt(nullptr)
	, _brush(nullptr)
	, _state(nullptr)
	, _interpolationMode(InterpolationMode::Linear)
	, _size(size)
	, _dirtyTransform(false)
	, _opacity(1.0f)
	, _pos()
	, _rotation(0.f)
	, _scale(1.f, 1.f)
	, _skew()
	, _style()
{
}

easy2d::Canvas::~Canvas()
{
	SafeRelease(_state);
	SafeRelease(_brush);
	SafeRelease(_rt);
	GC::release(_outputImage);
}

easy2d::Image* easy2d::Canvas::getOutputImage() const
{
	return _outputImage;
}

void easy2d::Canvas::beginDraw()
{
	HRESULT hr = S_OK;
	if (!_rt)
	{
		ID2D1BitmapRenderTarget* rt = nullptr;
		hr = Renderer::getRenderTarget()->CreateCompatibleRenderTarget(
			reinterpret_cast<const D2D1_SIZE_F&>(_size),
			&rt
		);

		ID2D1SolidColorBrush* brush = nullptr;
		if (SUCCEEDED(hr))
		{
			hr = rt->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::White),
				&brush
			);

			if (SUCCEEDED(hr))
			{
				_brush = brush;
				_brush->AddRef();
			}
		}

		if (SUCCEEDED(hr))
		{
			ID2D1Bitmap* bitmap = nullptr;
			hr = rt->GetBitmap(&bitmap);

			if (SUCCEEDED(hr))
			{
				_rt = rt;
				_rt->AddRef();

				if (_outputImage)
				{
					_outputImage->resetBitmap(bitmap);
				}
				else
				{
					_outputImage = gcnew Image(bitmap);
					GC::retain(_outputImage);
				}
			}
			SafeRelease(bitmap);
		}
		SafeRelease(brush);
		SafeRelease(rt);

		if (FAILED(hr))
		{
			E2D_ERROR("Canvas beginDraw failed! ERR_CODE=%#X", hr);
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = Renderer::getID2D1Factory()->CreateDrawingStateBlock(&_state);
		if (SUCCEEDED(hr))
		{
			_rt->SaveDrawingState(_state);
			_rt->BeginDraw();
		}
		else
		{
			E2D_ERROR("Canvas create drawing state block failed! ERR_CODE=%#X", hr);
		}
	}
}

void easy2d::Canvas::endDraw()
{
	_rt->EndDraw();

	if (_state)
	{
		_rt->RestoreDrawingState(_state);
	}
}

void easy2d::Canvas::_updateTransform()
{
	if (_dirtyTransform)
	{
		Matrix32 transform = Matrix32::scaling(_scale.x, _scale.y)
			* Matrix32::skewing(_skew.x, _skew.y)
			* Matrix32::rotation(_rotation)
			* Matrix32::translation(_pos.x, _pos.y);
		_rt->SetTransform(reinterpret_cast<const D2D1_MATRIX_3X2_F&>(transform));
		_dirtyTransform = false;
	}
}

void easy2d::Canvas::drawShape(Shape* shape)
{
	_updateTransform();
	_brush->SetOpacity(_opacity);
	if (_style.mode == DrawingStyle::Mode::Fill || _style.mode == DrawingStyle::Mode::Round)
	{
		ID2D1StrokeStyle* strokeStyle = nullptr;
		switch (_style.lineJoin)
		{
		case LineJoin::Miter:
			strokeStyle = Renderer::getMiterID2D1StrokeStyle();
			break;
		case LineJoin::Bevel:
			strokeStyle = Renderer::getBevelID2D1StrokeStyle();
			break;
		case LineJoin::Round:
			strokeStyle = Renderer::getRoundID2D1StrokeStyle();
			break;
		}
		_brush->SetColor(reinterpret_cast<const D2D1_COLOR_F&>(_style.strokeColor));
		_rt->DrawGeometry(shape->_geo, _brush, _style.strokeWidth, strokeStyle);
	}

	if (_style.mode == DrawingStyle::Mode::Fill || _style.mode == DrawingStyle::Mode::Solid)
	{
		_brush->SetColor(reinterpret_cast<const D2D1_COLOR_F&>(_style.fillColor));
		_rt->FillGeometry(shape->_geo, _brush);
	}
}

void easy2d::Canvas::drawImage(Image* image, const Point& pos, const Rect& cropRect)
{
	drawImage(image, Rect(pos, image->getSize()), cropRect);
}

void easy2d::Canvas::drawImage(Image* image, const Rect& destRect, const Rect& cropRect)
{
	_updateTransform();
	const Rect* srcRect = nullptr;
	if (!cropRect.isEmpty())
	{
		srcRect = &cropRect;
	}

	_rt->DrawBitmap(
		image->getBitmap(),
		reinterpret_cast<const D2D1_RECT_F&>(destRect),
		_opacity,
		(_interpolationMode == InterpolationMode::Nearest) ? D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR : D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
		reinterpret_cast<const D2D1_RECT_F*>(srcRect)
	);
}

void easy2d::Canvas::drawImage(KeyFrame* frame, const Point& pos)
{
	auto image = frame->getImage();
	drawImage(image, Rect(pos, image->getSize()), frame->getCropRect());
}

void easy2d::Canvas::drawImage(KeyFrame* frame, const Rect& destRect)
{
	auto image = frame->getImage();
	drawImage(image, destRect, frame->getCropRect());
}

void easy2d::Canvas::drawText(TextLayout* layout, const Point& pos)
{
	_updateTransform();
	_brush->SetOpacity(_opacity);
	Renderer::DrawTextLayout(layout, _style, pos, _rt, _brush);
}

void easy2d::Canvas::drawText(const String& text, const Point& pos, const TextStyle& style)
{
	TextLayout layout(text, style);
	drawText(&layout, pos);
}

void easy2d::Canvas::clear()
{
	_rt->Clear();
}

void easy2d::Canvas::clear(const Color& color)
{
	_rt->Clear(reinterpret_cast<const D2D1_COLOR_F&>(color));
}

easy2d::Color easy2d::Canvas::getFillColor() const
{
	return _style.fillColor;
}

easy2d::Color easy2d::Canvas::getStrokeColor() const
{
	return _style.strokeColor;
}

float easy2d::Canvas::getStrokeWidth() const
{
	return _style.strokeWidth;
}

void easy2d::Canvas::setFillColor(Color fillColor)
{
	_style.fillColor = fillColor;
}

void easy2d::Canvas::setStrokeColor(Color strokeColor)
{
	_style.strokeColor = strokeColor;
}

void easy2d::Canvas::setStrokeWidth(float strokeWidth)
{
	_style.strokeWidth = strokeWidth;
}

void easy2d::Canvas::setLineJoin(LineJoin lineJoin)
{
	_style.lineJoin = lineJoin;
}

easy2d::DrawingStyle::Mode easy2d::Canvas::getDrawingMode() const
{
	return _style.mode;
}

void easy2d::Canvas::setDrawingMode(DrawingStyle::Mode mode)
{
	_style.mode = mode;
}

easy2d::DrawingStyle easy2d::Canvas::getDrawingStyle() const
{
	return _style;
}

void easy2d::Canvas::setDrawingStyle(DrawingStyle style)
{
	_style = style;
}

float easy2d::Canvas::getOpacity() const
{
	return _opacity;
}

void easy2d::Canvas::setOpacity(float opacity)
{
	_opacity = max(min(opacity, 1.f), 0.f);
}

easy2d::Point easy2d::Canvas::getPos() const
{
	return _pos;
}

void easy2d::Canvas::setPos(const Point& point)
{
	if (_pos != point)
	{
		_pos = point;
		_dirtyTransform = true;
	}
}

void easy2d::Canvas::movePos(const Vector2& point)
{
	setPos(getPos() + point);
}

float easy2d::Canvas::getRotation() const
{
	return _rotation;
}

void easy2d::Canvas::setRotation(float rotation)
{
	if (_rotation != rotation)
	{
		_rotation = rotation;
		_dirtyTransform = true;
	}
}

easy2d::Vector2 easy2d::Canvas::getScale() const
{
	return _scale;
}

void easy2d::Canvas::setScale(const Vector2& scale)
{
	if (_scale != scale)
	{
		_scale = scale;
		_dirtyTransform = true;
	}
}

easy2d::Vector2 easy2d::Canvas::getSkew() const
{
	return _skew;
}

void easy2d::Canvas::setSkew(const Vector2& skew)
{
	if (_skew != skew)
	{
		_skew = skew;
		_dirtyTransform = true;
	}
}

easy2d::Matrix32 easy2d::Canvas::getTransform() const
{
	Matrix32 transform;
	_rt->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(&transform));
	return transform;
}

easy2d::InterpolationMode easy2d::Canvas::getInterpolationMode() const
{
	return _interpolationMode;
}

void easy2d::Canvas::setInterpolationMode(InterpolationMode mode)
{
	_interpolationMode = mode;
}
