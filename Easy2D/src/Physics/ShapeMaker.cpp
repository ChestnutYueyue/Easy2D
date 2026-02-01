#include <easy2d/e2dshape.h>

easy2d::ShapeMaker::ShapeMaker()
	: _pathOpen(false)
	, _geo(nullptr)
	, _sink(nullptr)
{
}

easy2d::ShapeMaker::~ShapeMaker()
{
}

easy2d::Shape* easy2d::ShapeMaker::combine(Shape* shape1, Shape* shape2, CombineMode mode, const Matrix32* matrix)
{
	// TODO: 实现OpenGL版本的形状合并
	// 暂时返回第一个形状
	if (shape1)
	{
		return shape1;
	}
	return shape2;
}

easy2d::Shape* easy2d::ShapeMaker::getShape() const
{
	// 创建多边形形状
	if (!_points.empty())
	{
		return gcnew Shape(Shape::Polygon, _points.data(), static_cast<int>(_points.size()));
	}
	return gcnew Shape(nullptr);
}

void* easy2d::ShapeMaker::getGeometry() const
{
	return _geo;
}

void easy2d::ShapeMaker::beginPath(const Point& point)
{
	_points.clear();
	_points.push_back(point);
	_pathOpen = true;
}

void easy2d::ShapeMaker::endPath(bool closed)
{
	_pathOpen = false;
	// 如果需要闭合路径，添加第一个点到末尾
	if (closed && _points.size() > 2)
	{
		_points.push_back(_points[0]);
	}
}

void easy2d::ShapeMaker::addLine(const Point& point)
{
	if (_pathOpen)
	{
		_points.push_back(point);
	}
}

void easy2d::ShapeMaker::addLines(std::initializer_list<Point> points)
{
	addLines(points.begin(), static_cast<int>(points.size()));
}

void easy2d::ShapeMaker::addLines(const Point* points, int count)
{
	if (_pathOpen && points && count > 0)
	{
		_points.insert(_points.end(), points, points + count);
	}
}

void easy2d::ShapeMaker::addBezier(const Point& point1, const Point& point2, const Point& point3)
{
	// TODO: 实现贝塞尔曲线细分并添加到点列表
	// 简化实现：只添加终点
	if (_pathOpen)
	{
		_points.push_back(point3);
	}
}

void easy2d::ShapeMaker::addArc(const Point& point, const Size& radius, float rotation, bool clockwise, bool smallSize)
{
	// TODO: 实现弧线细分并添加到点列表
	// 简化实现：只添加终点
	if (_pathOpen)
	{
		_points.push_back(point);
	}
}

void easy2d::ShapeMaker::openSink()
{
	// OpenGL版本不需要sink
	_pathOpen = true;
}

void easy2d::ShapeMaker::closeSink()
{
	_pathOpen = false;
}
