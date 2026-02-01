#define NOMINMAX
#include <easy2d/e2dshape.h>
#include <cmath>
#include <algorithm>

// 辅助函数：计算向量长度
static float VectorLength(const easy2d::Vector2& v)
{
	return std::sqrt(v.x * v.x + v.y * v.y);
}

// 辅助函数：归一化向量
static easy2d::Vector2 VectorNormalize(const easy2d::Vector2& v)
{
	float len = VectorLength(v);
	if (len > 0.0f)
	{
		return easy2d::Vector2(v.x / len, v.y / len);
	}
	return v;
}

easy2d::Shape::LineType easy2d::Shape::Line;
easy2d::Shape::RectType easy2d::Shape::Rect;
easy2d::Shape::RoundedRectType easy2d::Shape::RoundedRect;
easy2d::Shape::CircleType easy2d::Shape::Circle;
easy2d::Shape::EllipseType easy2d::Shape::Ellipse;
easy2d::Shape::PolygonType easy2d::Shape::Polygon;

easy2d::Shape::Shape(void* geo)
	: _type(Type::Unknown)
	, _geo(geo)
{
}

easy2d::Shape::~Shape()
{
}

void easy2d::Shape::resetGeometry(void* geo)
{
	_geo = geo;
}

easy2d::Shape::Shape(LineType, Point begin, Point end)
	: _type(Type::Line)
	, _geo(nullptr)
	, _begin(begin)
	, _end(end)
{
}

easy2d::Shape::Shape(RectType, const easy2d::Rect& rect)
	: _type(Type::Rect)
	, _geo(nullptr)
	, _rect(rect)
{
}

easy2d::Shape::Shape(RoundedRectType, const easy2d::Rect& rect, const Vector2& radius)
	: _type(Type::RoundedRect)
	, _geo(nullptr)
	, _rect(rect)
	, _radius(radius)
{
}

easy2d::Shape::Shape(CircleType, const Point& center, float radius)
	: _type(Type::Ellipse)
	, _geo(nullptr)
	, _center(center)
	, _radius(radius, radius)
{
}

easy2d::Shape::Shape(EllipseType, const Point& center, const Vector2& radius)
	: _type(Type::Ellipse)
	, _geo(nullptr)
	, _center(center)
	, _radius(radius)
{
}

easy2d::Shape::Shape(PolygonType, std::initializer_list<Point> vertices)
	: _type(Type::Polygon)
	, _geo(nullptr)
	, _vertices(vertices)
{
}

easy2d::Shape::Shape(PolygonType, const Point* vertices, int count)
	: _type(Type::Polygon)
	, _geo(nullptr)
{
	if (count > 0 && vertices)
	{
		_vertices.assign(vertices, vertices + count);
	}
}

easy2d::Rect easy2d::Shape::getBoundingBox(const Matrix32* transform) const
{
	switch (_type)
	{
	case Type::Rect:
	case Type::RoundedRect:
		return _rect;
	case Type::Ellipse:
		return easy2d::Rect(
			Point(_center.x - _radius.x, _center.y - _radius.y),
			Point(_center.x + _radius.x, _center.y + _radius.y)
		);
	case Type::Line:
		return easy2d::Rect(
			Point(std::min(_begin.x, _end.x), std::min(_begin.y, _end.y)),
			Point(std::max(_begin.x, _end.x), std::max(_begin.y, _end.y))
		);
	case Type::Polygon:
		if (!_vertices.empty())
		{
			float minX = _vertices[0].x;
			float maxX = _vertices[0].x;
			float minY = _vertices[0].y;
			float maxY = _vertices[0].y;
			for (const auto& p : _vertices)
			{
				minX = std::min(minX, p.x);
				maxX = std::max(maxX, p.x);
				minY = std::min(minY, p.y);
				maxY = std::max(maxY, p.y);
			}
			return easy2d::Rect(Point(minX, minY), Point(maxX, maxY));
		}
		return easy2d::Rect();
	default:
		return easy2d::Rect();
	}
}

bool easy2d::Shape::containsPoint(const Point& point, const Matrix32* transform) const
{
	// 简化的点包含检测
	easy2d::Rect bounds = getBoundingBox(transform);
	return bounds.containsPoint(point);
}

float easy2d::Shape::getLength() const
{
	switch (_type)
	{
	case Type::Line:
		return VectorLength(_end - _begin);
	case Type::Rect:
		return 2.0f * (_rect.getWidth() + _rect.getHeight());
	case Type::Circle:
	case Type::Ellipse:
		// 椭圆周长近似公式
		return 3.14159265359f * (3.0f * (_radius.x + _radius.y) - 
			sqrtf((3.0f * _radius.x + _radius.y) * (_radius.x + 3.0f * _radius.y)));
	default:
		return 0.0f;
	}
}

float easy2d::Shape::computeArea() const
{
	switch (_type)
	{
	case Type::Rect:
	case Type::RoundedRect:
		return _rect.getWidth() * _rect.getHeight();
	case Type::Ellipse:
		return 3.14159265359f * _radius.x * _radius.y;
	case Type::Circle:
		return 3.14159265359f * _radius.x * _radius.x;
	case Type::Polygon:
	{
		// 使用鞋带公式计算多边形面积
		float area = 0.0f;
		int n = static_cast<int>(_vertices.size());
		for (int i = 0; i < n; ++i)
		{
			int j = (i + 1) % n;
			area += _vertices[i].x * _vertices[j].y;
			area -= _vertices[j].x * _vertices[i].y;
		}
		return fabsf(area) * 0.5f;
	}
	default:
		return 0.0f;
	}
}

bool easy2d::Shape::computePointAtLength(float length, Point& point, Vector2& tangent) const
{
	// 简化实现，返回起点
	if (_type == Type::Line)
	{
		point = _begin;
		tangent = VectorNormalize(_end - _begin);
		return true;
	}
	return false;
}
