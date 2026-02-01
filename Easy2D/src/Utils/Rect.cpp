#include <easy2d/e2dmath.h>

namespace easy2d {

// Rect 构造函数
Rect::Rect() = default;
Rect::Rect(const Point& lt, const Point& rb) : leftTop(lt), rightBottom(rb) {}
Rect::Rect(const Point& pos, const Size& size) { setRect(pos, size); }
Rect::Rect(const Rect& other) : leftTop(other.leftTop), rightBottom(other.rightBottom) {}

// Rect 运算符
Rect& Rect::operator= (const Rect& other)
{
    leftTop = other.leftTop;
    rightBottom = other.rightBottom;
    return *this;
}

bool Rect::operator== (const Rect& rect) const { return leftTop == rect.leftTop && rightBottom == rect.rightBottom; }

// Rect 方法
bool Rect::isEmpty() const { return leftTop.isEmpty() && rightBottom.isEmpty(); }

void Rect::setRect(const Point& pos, const Size& size)
{
    leftTop = pos;
    rightBottom = Point{ pos.x + size.width, pos.y + size.height };
}

bool Rect::containsPoint(const Point& point) const
{
    return point.x >= leftTop.x && point.x <= rightBottom.x && point.y >= leftTop.y && point.y <= rightBottom.y;
}

bool Rect::intersects(const Rect& rect) const
{
    return !(rightBottom.x < rect.leftTop.x || rect.rightBottom.x < leftTop.x ||
            rightBottom.y < rect.leftTop.y || rect.rightBottom.y < leftTop.y);
}

float Rect::getWidth() const { return rightBottom.x - leftTop.x; }
float Rect::getHeight() const { return rightBottom.y - leftTop.y; }
Size Rect::getSize() const { return Size{ getWidth(), getHeight() }; }
Vector2 Rect::getLeftTop() const { return leftTop; }
Vector2 Rect::getRightBottom() const { return rightBottom; }
Vector2 Rect::getRightTop() const { return Vector2{ getRight(), getTop() }; }
Vector2 Rect::getLeftBottom() const { return Vector2{ getLeft(), getBottom() }; }
float Rect::getLeft() const { return leftTop.x; }
float Rect::getTop() const { return leftTop.y; }
float Rect::getRight() const { return rightBottom.x; }
float Rect::getBottom() const { return rightBottom.y; }

}
