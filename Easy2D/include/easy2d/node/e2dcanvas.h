#pragma once
#include <easy2d/core/e2dcommon.h>
#include <easy2d/node/e2dtext.h>
#include <easy2d/node/e2dshape.h>
#include <easy2d/action/e2daction.h>

namespace easy2d
{

class CanvasBrush;

// 画布
class Canvas
	: public Object
{
	friend class CanvasBrush;

public:
	Canvas(const Size& size);

	virtual ~Canvas();

	// 开始绘图
	CanvasBrush* beginDraw();

	// 结束绘图
	void endDraw();

	// 获取画布图像
	Image* getOutputImage() const;

protected:
	ID2D1RenderTarget* _rt;
	ID2D1DrawingStateBlock* _state;
	Image* _outputImage;
	Size _size;
};


// 画布画刷
class CanvasBrush
	: public Object
{
	friend class Canvas;

public:
	// 清空画布
	void clear();

	// 指定颜色清空画布
	void clear(const Color& color);

	// 绘制形状
	void drawShape(
		Shape* shape
	);

	// 绘制图片
	void drawImage(
		Image* image,					/* 图像 */
		const Point& pos = Point(),		/* 绘制位置 */
		const Rect& cropRect = Rect{}	/* 裁剪矩形 */
	);

	// 绘制图片
	void drawImage(
		Image* image,					/* 图像 */
		const Rect& destRect,			/* 绘制区域 */
		const Rect& cropRect = Rect{}	/* 裁剪矩形 */
	);

	// 绘制关键帧
	void drawImage(
		KeyFrame* frame,				/* 关键帧 */
		const Point& pos = Point()		/* 绘制位置 */
	);

	// 绘制关键帧
	void drawImage(
		KeyFrame* frame,				/* 关键帧 */
		const Rect& destRect			/* 绘制区域 */
	);

	// 绘制文本
	void drawText(
		TextLayout* layout,
		const Point& pos = Point()
	);

	// 绘制文本
	void drawText(
		const String& text,
		const Point& pos = Point(),
		const TextStyle& style = TextStyle()
	);

	// 获取填充颜色
	Color getFillColor() const;

	// 设置填充颜色
	void setFillColor(
		Color fillColor
	);

	// 获取线条颜色
	Color getStrokeColor() const;

	// 设置线条颜色
	void setStrokeColor(
		Color strokeColor
	);

	// 获取线条宽度
	float getStrokeWidth() const;

	// 设置线条宽度
	void setStrokeWidth(
		float strokeWidth
	);

	// 设置线条相交样式
	void setLineJoin(
		LineJoin lineJoin
	);

	// 获取绘图模式
	DrawingStyle::Mode getDrawingMode() const;

	// 设置绘图模式
	void setDrawingMode(DrawingStyle::Mode mode);

	// 获取绘图样式
	DrawingStyle getDrawingStyle() const;

	// 设置绘图样式
	void setDrawingStyle(DrawingStyle style);

	// 获取透明度
	float getOpacity() const;

	// 设置透明度（范围：0 ~ 1）
	void setOpacity(float opacity);

	// 获取坐标
	Point getPos() const;

	// 设置坐标
	void setPos(
		const Point& point
	);

	// 移动画笔
	void movePos(
		const Vector2& point
	);

	// 获取旋转角度
	float getRotation() const;

	// 设置旋转角度
	void setRotation(
		float rotation
	);

	// 获取缩放比例
	Vector2 getScale() const;

	// 设置缩放比例
	void setScale(
		const Vector2& scale
	);

	// 获取节点倾斜角度
	Vector2 getSkew() const;

	// 设置倾斜角度
	void setSkew(
		const Vector2& skew
	);

	// 获取画笔变换矩阵
	Matrix32 getTransform() const;

	// 获取绘制图像时的像素插值方式
	InterpolationMode getInterpolationMode() const;

	// 设置绘制图像时的像素插值方式
	void setInterpolationMode(InterpolationMode mode);

protected:
	CanvasBrush(Canvas* canvas);

	virtual ~CanvasBrush();

	void _updateTransform();

protected:
	bool _dirtyTransform;
	ID2D1RenderTarget* _rt;
	ID2D1SolidColorBrush* _brush;
	InterpolationMode _interpolationMode;
	float _opacity;
	float _rotation;
	Point _pos;
	Vector2 _scale;
	Vector2 _skew;
	DrawingStyle _style;
};

}
