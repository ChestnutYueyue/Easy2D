#include <easy2d/e2daction.h>
#include <easy2d/e2dnode.h>

easy2d::KeyFrame::KeyFrame(Image* image)
	: _image(image)
	, _cropRect()
{
	GC::retain(image);
}

easy2d::KeyFrame::KeyFrame(Image* image, const Rect& cropRect)
	: _image(image)
	, _cropRect()
{
	GC::retain(image);
	setCropRect(cropRect);
}

easy2d::KeyFrame::~KeyFrame()
{
	GC::release(_image);
}

easy2d::Image* easy2d::KeyFrame::getImage() const
{
	return _image;
}

void easy2d::KeyFrame::setImage(Image* image)
{
	GC::release(_image);
	_image = image;
	GC::retain(_image);
}

easy2d::Rect easy2d::KeyFrame::getCropRect() const
{
	return _cropRect;
}

void easy2d::KeyFrame::setCropRect(const Rect& cropRect)
{
	_cropRect.leftTop.x = std::min(std::max(cropRect.leftTop.x, 0.0f), _image->getWidth());
	_cropRect.leftTop.y = std::min(std::max(cropRect.leftTop.y, 0.0f), _image->getHeight());
	_cropRect.rightBottom.x = std::min(std::max(cropRect.rightBottom.x, 0.0f), _image->getWidth());
	_cropRect.rightBottom.y = std::min(std::max(cropRect.rightBottom.y, 0.0f), _image->getHeight());
}

float easy2d::KeyFrame::getWidth() const
{
	return _cropRect.getWidth();
}

float easy2d::KeyFrame::getHeight() const
{
	return _cropRect.getHeight();
}

easy2d::Size easy2d::KeyFrame::getSize() const
{
	return _cropRect.getSize();
}
