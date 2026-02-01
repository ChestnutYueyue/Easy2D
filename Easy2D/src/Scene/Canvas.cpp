#define NOMINMAX
#include <easy2d/e2dcanvas.h>
#include <easy2d/e2dbase.h>
#include <easy2d/GLRenderer.h>
#include <easy2d/GLFrameBuffer.h>
#include <easy2d/GLTexture.h>

namespace easy2d
{

Canvas::Canvas(const Size& size)
	: _frameBuffer(nullptr)
	, _state(nullptr)
	, _outputImage(nullptr)
	, _size(size)
{
}

Canvas::~Canvas()
{
	if (_frameBuffer)
	{
		delete static_cast<GLFrameBuffer*>(_frameBuffer);
		_frameBuffer = nullptr;
	}
	GC::release(_outputImage);
}

Image* Canvas::getOutputImage() const
{
	return _outputImage;
}

CanvasBrush* Canvas::beginDraw()
{
	// 创建帧缓冲（如果还没有创建）
	if (!_frameBuffer)
	{
		_frameBuffer = new GLFrameBuffer();
		if (!static_cast<GLFrameBuffer*>(_frameBuffer)->create(static_cast<int>(_size.width), static_cast<int>(_size.height)))
		{
			E2D_ERROR("Canvas beginDraw failed! Failed to create framebuffer");
			delete static_cast<GLFrameBuffer*>(_frameBuffer);
			_frameBuffer = nullptr;
			return nullptr;
		}

		// 创建输出图像
		GLTexture* texture = static_cast<GLFrameBuffer*>(_frameBuffer)->getColorTexture();
		if (_outputImage)
		{
			_outputImage->resetTexture(texture);
		}
		else
		{
			_outputImage = gcnew Image(texture);
			_outputImage->retain();
		}
	}

	// 绑定帧缓冲
	static_cast<GLFrameBuffer*>(_frameBuffer)->bind();

	// 清空帧缓冲
	static_cast<GLFrameBuffer*>(_frameBuffer)->clear(0.0f, 0.0f, 0.0f, 0.0f);

	return gcnew CanvasBrush(this);
}

void Canvas::endDraw()
{
	if (_frameBuffer)
	{
		static_cast<GLFrameBuffer*>(_frameBuffer)->unbind();
	}
}

//
// CanvasBrush
//

CanvasBrush::CanvasBrush(Canvas* canvas)
	: _dirtyTransform(false)
	, _frameBuffer(canvas->_frameBuffer)
	, _brush(nullptr)
	, _interpolationMode(InterpolationMode::Linear)
	, _opacity(1.0f)
	, _rotation(0.f)
	, _pos()
	, _scale(1.f, 1.f)
	, _skew()
	, _style()
{
}

CanvasBrush::~CanvasBrush()
{
}

void CanvasBrush::_updateTransform()
{
	if (_dirtyTransform)
	{
		Matrix32 transform = Matrix32::scaling(_scale.x, _scale.y)
			* Matrix32::skewing(_skew.x, _skew.y)
			* Matrix32::rotation(_rotation)
			* Matrix32::translation(_pos.x, _pos.y);
		
		// 设置OpenGL模型矩阵
		E2D_GL_RENDERER.setModelMatrix(transform);
		_dirtyTransform = false;
	}
}

void CanvasBrush::drawShape(Shape* shape)
{
	_updateTransform();
	
	if (!shape)
	{
		return;
	}
	
	// 根据形状类型调用相应的绘制方法
	// 注意：这里使用简化的实现，实际应该将Shape转换为OpenGL几何体
	Rect bounds = shape->getBoundingBox();
	
	if (_style.mode == DrawingStyle::Mode::Fill || _style.mode == DrawingStyle::Mode::Round)
	{
		E2D_GL_RENDERER.setOpacity(_opacity);
		E2D_GL_RENDERER.drawRectOutline(bounds, _style.strokeColor, _style.strokeWidth);
	}
	
	if (_style.mode == DrawingStyle::Mode::Fill || _style.mode == DrawingStyle::Mode::Solid)
	{
		E2D_GL_RENDERER.setOpacity(_opacity);
		E2D_GL_RENDERER.drawFilledRect(bounds, _style.fillColor);
	}
}

void CanvasBrush::drawImage(Image* image, const Point& pos, const Rect& cropRect)
{
	drawImage(image, Rect(pos, image->getSize()), cropRect);
}

void CanvasBrush::drawImage(Image* image, const Rect& destRect, const Rect& cropRect)
{
	_updateTransform();
	
	GLTexture* texture = image->getTexture();
	if (!texture)
	{
		return;
	}

	const Rect* srcRect = nullptr;
	if (!cropRect.isEmpty())
	{
		srcRect = &cropRect;
	}

	// 设置插值模式
	if (_interpolationMode == InterpolationMode::Nearest)
	{
		texture->setFilterMode(GL_NEAREST, GL_NEAREST);
	}
	else
	{
		texture->setFilterMode(GL_LINEAR, GL_LINEAR);
	}

	E2D_GL_RENDERER.setOpacity(_opacity);
	E2D_GL_RENDERER.drawTexture(texture, destRect, srcRect, Color::White);
}

void CanvasBrush::drawImage(KeyFrame* frame, const Point& pos)
{
	auto image = frame->getImage();
	drawImage(image, Rect(pos, image->getSize()), frame->getCropRect());
}

void CanvasBrush::drawImage(KeyFrame* frame, const Rect& destRect)
{
	auto image = frame->getImage();
	drawImage(image, destRect, frame->getCropRect());
}

void CanvasBrush::drawText(TextLayout* layout, const Point& pos)
{
	_updateTransform();
	E2D_GL_RENDERER.setOpacity(_opacity);
	E2D_GL_RENDERER.drawTextLayout(layout, pos, _style);
}

void CanvasBrush::drawText(const String& text, const Point& pos, const TextStyle& style)
{
	TextLayout layout(text, style);
	drawText(&layout, pos);
}

void CanvasBrush::clear()
{
	if (_frameBuffer)
	{
		static_cast<GLFrameBuffer*>(_frameBuffer)->clear(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

void CanvasBrush::clear(const Color& color)
{
	if (_frameBuffer)
	{
		static_cast<GLFrameBuffer*>(_frameBuffer)->clear(color.r, color.g, color.b, color.a);
	}
}

Color CanvasBrush::getFillColor() const
{
	return _style.fillColor;
}

Color CanvasBrush::getStrokeColor() const
{
	return _style.strokeColor;
}

float CanvasBrush::getStrokeWidth() const
{
	return _style.strokeWidth;
}

void CanvasBrush::setFillColor(Color fillColor)
{
	_style.fillColor = fillColor;
}

void CanvasBrush::setStrokeColor(Color strokeColor)
{
	_style.strokeColor = strokeColor;
}

void CanvasBrush::setStrokeWidth(float strokeWidth)
{
	_style.strokeWidth = strokeWidth;
}

void CanvasBrush::setLineJoin(LineJoin lineJoin)
{
	_style.lineJoin = lineJoin;
}

DrawingStyle::Mode CanvasBrush::getDrawingMode() const
{
	return _style.mode;
}

void CanvasBrush::setDrawingMode(DrawingStyle::Mode mode)
{
	_style.mode = mode;
}

DrawingStyle CanvasBrush::getDrawingStyle() const
{
	return _style;
}

void CanvasBrush::setDrawingStyle(DrawingStyle style)
{
	_style = style;
}

float CanvasBrush::getOpacity() const
{
	return _opacity;
}

void CanvasBrush::setOpacity(float opacity)
{
	_opacity = std::max(std::min(opacity, 1.0f), 0.0f);
}

Point CanvasBrush::getPos() const
{
	return _pos;
}

void CanvasBrush::setPos(const Point& point)
{
	if (_pos != point)
	{
		_pos = point;
		_dirtyTransform = true;
	}
}

void CanvasBrush::movePos(const Vector2& point)
{
	setPos(getPos() + point);
}

float CanvasBrush::getRotation() const
{
	return _rotation;
}

void CanvasBrush::setRotation(float rotation)
{
	if (_rotation != rotation)
	{
		_rotation = rotation;
		_dirtyTransform = true;
	}
}

Vector2 CanvasBrush::getScale() const
{
	return _scale;
}

void CanvasBrush::setScale(const Vector2& scale)
{
	if (_scale != scale)
	{
		_scale = scale;
		_dirtyTransform = true;
	}
}

Vector2 CanvasBrush::getSkew() const
{
	return _skew;
}

void CanvasBrush::setSkew(const Vector2& skew)
{
	if (_skew != skew)
	{
		_skew = skew;
		_dirtyTransform = true;
	}
}

Matrix32 CanvasBrush::getTransform() const
{
	return Matrix32::scaling(_scale.x, _scale.y)
		* Matrix32::skewing(_skew.x, _skew.y)
		* Matrix32::rotation(_rotation)
		* Matrix32::translation(_pos.x, _pos.y);
}

InterpolationMode CanvasBrush::getInterpolationMode() const
{
	return _interpolationMode;
}

void CanvasBrush::setInterpolationMode(InterpolationMode mode)
{
	_interpolationMode = mode;
}

}
