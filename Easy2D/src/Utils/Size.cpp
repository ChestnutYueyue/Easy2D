#include <easy2d/e2dmath.h>

namespace easy2d {

// Size 构造函数
Size::Size() : width(0), height(0) {}
Size::Size(float width, float height) : width(width), height(height) {}
Size::Size(const Size& other) : width(other.width), height(other.height) {}

// Size 运算符
Size Size::operator + (Size const& size) const { return Size(width + size.width, height + size.height); }
Size Size::operator - (Size const& size) const { return Size(width - size.width, height - size.height); }
Size Size::operator * (float const& size) const { return Size(width * size, height * size); }
Size Size::operator / (float const& size) const { return Size(width / size, height / size); }
Size Size::operator - () const { return Size(-width, -height); }
bool Size::operator== (const Size& size) const { return width == size.width && height == size.height; }
bool Size::operator!= (const Size& size) const { return !operator==(size); }

// Size 类型转换
Size::operator easy2d::Point() const { return Point(width, height); }
Size::operator glm::vec2() const { return glm::vec2(width, height); }

// Size 方法
bool Size::isEmpty() const { return !width && !height; }

}
