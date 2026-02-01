#include <easy2d/e2dmath.h>

namespace easy2d {

// Matrix32 构造函数
Matrix32::Matrix32()
    : _11(1.f), _12(0.f)
    , _21(0.f), _22(1.f)
    , _31(0.f), _32(0.f)
{
}

Matrix32::Matrix32(float _11, float _12, float _21, float _22, float _31, float _32)
    : _11(_11), _12(_12), _21(_21), _22(_22), _31(_31), _32(_32)
{
}

Matrix32::Matrix32(Matrix32 const& other)
    : _11(other._11), _12(other._12)
    , _21(other._21), _22(other._22)
    , _31(other._31), _32(other._32)
{
}

Matrix32::Matrix32(const glm::mat3& mat)
{
    _11 = mat[0][0]; _12 = mat[0][1];
    _21 = mat[1][0]; _22 = mat[1][1];
    _31 = mat[2][0]; _32 = mat[2][1];
}

// Matrix32 运算符
float Matrix32::operator [](unsigned int index) const { return m[index]; }

// Matrix32 方法
void Matrix32::identity()
{
    _11 = 1.f; _12 = 0.f;
    _21 = 0.f; _22 = 1.f;
    _31 = 0.f; _32 = 0.f;
}

Vector2 Matrix32::transform(const Vector2& v) const
{
    return Vector2(
        v.x * _11 + v.y * _21 + _31,
        v.x * _12 + v.y * _22 + _32
    );
}

Point Matrix32::transformPoint(const Point& p) const
{
    return Point(
        p.x * _11 + p.y * _21 + _31,
        p.x * _12 + p.y * _22 + _32
    );
}

Rect Matrix32::transform(const Rect& rect) const
{
    Vector2 top_left = transform(rect.getLeftTop());
    Vector2 top_right = transform(rect.getRightTop());
    Vector2 bottom_left = transform(rect.getLeftBottom());
    Vector2 bottom_right = transform(rect.getRightBottom());

    float left = std::min(std::min(top_left.x, top_right.x), std::min(bottom_left.x, bottom_right.x));
    float right = std::max(std::max(top_left.x, top_right.x), std::max(bottom_left.x, bottom_right.x));
    float top = std::min(std::min(top_left.y, top_right.y), std::min(bottom_left.y, bottom_right.y));
    float bottom = std::max(std::max(top_left.y, top_right.y), std::max(bottom_left.y, bottom_right.y));

    return Rect{ Point{left, top}, Point{right, bottom} };
}

void Matrix32::translate(float x, float y)
{
    _31 += _11 * x + _21 * y;
    _32 += _12 * x + _22 * y;
}

void Matrix32::translate(const Vector2& v)
{
    translate(v.x, v.y);
}

float Matrix32::determinant() const
{
    return (_11 * _22) - (_12 * _21);
}

bool Matrix32::isIdentity() const
{
    return _11 == 1.f && _12 == 0.f &&
        _21 == 0.f && _22 == 1.f &&
        _31 == 0.f && _32 == 0.f;
}

bool Matrix32::isInvertible() const
{
    return 0 != determinant();
}

glm::mat3 Matrix32::toMat3() const
{
    return glm::mat3(
        _11, _12, 0.0f,
        _21, _22, 0.0f,
        _31, _32, 1.0f
    );
}

glm::mat4 Matrix32::toMat4() const
{
	return glm::mat4(
		_11, _12, 0.0f, 0.0f,
		_21, _22, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		_31, _32, 0.0f, 1.0f
	);
}

// 静态方法
Matrix32 Matrix32::translation(float x, float y)
{
    return Matrix32(
        1.f, 0.f,
        0.f, 1.f,
        x, y
    );
}

Matrix32 Matrix32::scaling(float x, float y, const Point& center)
{
    return Matrix32(
        x, 0.f,
        0.f, y,
        center.x - x * center.x,
        center.y - y * center.y
    );
}

Matrix32 Matrix32::rotation(float angle, const Point& center)
{
    float radians = angle * math::constants::PI_F / 180.f;
    float s = glm::sin(radians);
    float c = glm::cos(radians);
    return Matrix32(
        c, s,
        -s, c,
        center.x * (1 - c) + center.y * s,
        center.y * (1 - c) - center.x * s
    );
}

Matrix32 Matrix32::skewing(float angle_x, float angle_y, const Point& center)
{
    float tx = glm::tan(angle_x * math::constants::PI_F / 180.f);
    float ty = glm::tan(angle_y * math::constants::PI_F / 180.f);
    return Matrix32(
        1.f, -ty,
        -tx, 1.f,
        center.y * tx, center.x * ty
    );
}

Matrix32 Matrix32::invert(Matrix32 const& matrix)
{
    float det = 1.f / matrix.determinant();

    return Matrix32(
        det * matrix._22,
        -det * matrix._12,
        -det * matrix._21,
        det * matrix._11,
        det * (matrix._21 * matrix._32 - matrix._22 * matrix._31),
        det * (matrix._12 * matrix._31 - matrix._11 * matrix._32)
    );
}

// 矩阵乘法
Matrix32 Matrix32::operator*(const Matrix32& other) const
{
    return Matrix32(
        _11 * other._11 + _12 * other._21,
        _11 * other._12 + _12 * other._22,
        _21 * other._11 + _22 * other._21,
        _21 * other._12 + _22 * other._22,
        _31 * other._11 + _32 * other._21 + other._31,
        _31 * other._12 + _32 * other._22 + other._32
    );
}

}
