#include <easy2d/e2dmath.h>

namespace easy2d {

// Point 构造函数
Point::Point() : x(0), y(0) {}
Point::Point(float x, float y) : x(x), y(y) {}
Point::Point(const Point& other) : x(other.x), y(other.y) {}
Point::Point(const glm::vec2& v) : x(v.x), y(v.y) {}

// Point 运算符
Point Point::operator + (Point const& point) const { return Point(x + point.x, y + point.y); }
Point Point::operator - (Point const& point) const { return Point(x - point.x, y - point.y); }
Point Point::operator * (float const& point) const { return Point(x * point, y * point); }
Point Point::operator / (float const& point) const { return Point(x / point, y / point); }
Point Point::operator - () const { return Point(-x, -y); }
bool Point::operator== (const Point& point) const { return x == point.x && y == point.y; }
bool Point::operator!= (const Point& point) const { return !operator==(point); }

// Point 类型转换
Point::operator easy2d::Size() const { return Size(x, y); }
Point::operator glm::vec2() const { return glm::vec2(x, y); }

// Point 方法
bool Point::isEmpty() const { return !x && !y; }
glm::vec2 Point::toVec2() const { return glm::vec2(x, y); }

// 静态方法
float Point::distance(const Point& p1, const Point& p2)
{
    return glm::distance(glm::vec2(p1.x, p1.y), glm::vec2(p2.x, p2.y));
}

}
