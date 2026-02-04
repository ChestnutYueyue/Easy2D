#pragma once
#include <cmath>
#include <easy2d/e2dmacros.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace easy2d {
namespace math {
namespace constants {
const auto PI_F = 3.141592653589793f;
const auto PI_F_2 = 1.570796326794896f;
const auto PI_F_X_2 = 6.283185307179586f;

const auto PI_D = 3.14159265358979323846;
const auto PI_D_2 = 1.57079632679489661923;
const auto PI_D_X_2 = 6.28318530717958647692;

// 预计算角度转弧度常量 (PI / 180)
const auto DEG_TO_RAD_F = 0.017453292519943295f;
const auto DEG_TO_RAD_D = 0.017453292519943295769236907684886;
// 预计算弧度转角度常量 (180 / PI)
const auto RAD_TO_DEG_F = 57.29577951308232f;
const auto RAD_TO_DEG_D = 57.295779513082320876798154814105;
} // namespace constants

inline int Abs(int val) { return ::abs(val); }

inline float Abs(float val) { return ::fabsf(val); }

inline double Abs(double val) { return ::fabs(val); }

inline float Sqrt(float val) { return ::sqrtf(val); }

inline double Sqrt(double val) { return ::sqrt(val); }

inline float Pow(float base, float exponent) { return ::powf(base, exponent); }

inline double Pow(double base, double exponent) {
  return ::pow(base, exponent);
}

inline int Sign(int val) { return val < 0 ? -1 : 1; }

inline float Sign(float val) { return val < 0 ? -1.f : 1.f; }

inline double Sign(double val) { return val < 0 ? -1.0 : 1.0; }

inline float Sin(float val) { return ::sinf(val * constants::DEG_TO_RAD_F); }

inline double Sin(double val) { return ::sin(val * constants::DEG_TO_RAD_D); }

inline float Cos(float val) { return ::cosf(val * constants::DEG_TO_RAD_F); }

inline double Cos(double val) { return ::cos(val * constants::DEG_TO_RAD_D); }

inline float Tan(float val) { return ::tanf(val * constants::DEG_TO_RAD_F); }

inline double Tan(double val) { return ::tan(val * constants::DEG_TO_RAD_D); }

inline float Asin(float val) { return ::asinf(val) * constants::RAD_TO_DEG_F; }

inline double Asin(double val) { return ::asin(val) * constants::RAD_TO_DEG_D; }

inline float Acos(float val) { return ::acosf(val) * constants::RAD_TO_DEG_F; }

inline double Acos(double val) { return ::acos(val) * constants::RAD_TO_DEG_D; }

inline float Atan(float val) { return ::atanf(val) * constants::RAD_TO_DEG_F; }

inline double Atan(double val) { return ::atan(val) * constants::RAD_TO_DEG_D; }

inline float Ceil(float val) { return ::ceil(val); }

inline double Ceil(double val) { return ::ceil(val); }

inline float Floor(float val) { return ::floor(val); }

inline double Floor(double val) { return ::floor(val); }
} // namespace math

class Size;

// 坐标
class Point {
public:
  float x; // X 坐标
  float y; // Y 坐标

public:
  Point();

  Point(float x, float y);

  Point(const Point &other);

  Point operator+(Point const &point) const;
  Point operator-(Point const &point) const;
  Point operator*(float const &point) const;
  Point operator/(float const &point) const;
  Point operator-() const;
  bool operator==(const Point &point) const;
  bool operator!=(const Point &point) const;

  operator easy2d::Size() const;

  bool isEmpty() const;

  // 判断两点间距离
  static float distance(const Point &, const Point &);
};

// 二维向量
using Vector2 = Point;

// 大小
class Size {
public:
  float width;  // 宽度
  float height; // 高度

public:
  Size();

  Size(float width, float height);

  Size(const Size &other);

  Size operator+(Size const &size) const;
  Size operator-(Size const &size) const;
  Size operator*(float const &size) const;
  Size operator/(float const &size) const;
  Size operator-() const;
  bool operator==(const Size &size) const;
  bool operator!=(const Size &size) const;

  operator easy2d::Point() const;

  bool isEmpty() const;
};

// 矩形
class Rect {
public:
  Point leftTop, rightBottom;

public:
  Rect();

  Rect(const Point &lt, const Point &rb);

  Rect(const Point &pos, const Size &size);

  Rect(const Rect &other);

  Rect &operator=(const Rect &other);

  bool operator==(const Rect &rect) const;

  bool isEmpty() const;

  // 设置矩形
  void setRect(const Point &pos, const Size &size);

  // 判断点是否在矩形内
  bool containsPoint(const Point &point) const;

  // 判断两矩形是否相交
  bool intersects(const Rect &rect) const;

  inline float getWidth() const { return rightBottom.x - leftTop.x; }

  inline float getHeight() const { return rightBottom.y - leftTop.y; }

  inline Size getSize() const { return Size{getWidth(), getHeight()}; }

  inline Vector2 getLeftTop() const { return leftTop; }

  inline Vector2 getRightBottom() const { return rightBottom; }

  inline Vector2 getRightTop() const { return Vector2{getRight(), getTop()}; }

  inline Vector2 getLeftBottom() const {
    return Vector2{getLeft(), getBottom()};
  }

  inline float getLeft() const { return leftTop.x; }

  inline float getTop() const { return leftTop.y; }

  inline float getRight() const { return rightBottom.x; }

  inline float getBottom() const { return rightBottom.y; }
};

// 二维变换矩阵 (使用4x4矩阵作为底层，基于glm，利用glm::translate/rotate/scale自动计算)
struct Matrix33 {
  glm::mat4 matrix;

  // 默认构造函数，初始化为单位矩阵
  Matrix33();

  // 从glm::mat4构造
  Matrix33(const glm::mat4 &m);

  // 从元素构造（仅使用2D仿射变换的6个参数）
  Matrix33(float m00, float m01, float m02, float m10, float m11, float m12);

  // 拷贝构造函数
  Matrix33(const Matrix33 &other);

  // 赋值运算符
  Matrix33 &operator=(const Matrix33 &other);

  // 矩阵乘法
  Matrix33 operator*(const Matrix33 &other) const;
  Matrix33 &operator*=(const Matrix33 &other);

  // 下标访问（返回矩阵元素的引用）
  float &operator()(int row, int col);
  float operator()(int row, int col) const;

  // 设置为单位矩阵
  void identity();

  // 变换向量
  Vector2 transform(const Vector2 &v) const;

  // 变换矩形
  Rect transform(const Rect &rect) const;

  // 平移
  void translate(float x, float y);
  void translate(const Vector2 &v);

  // 计算行列式
  float determinant() const;

  // 判断是否为单位矩阵
  bool isIdentity() const;

  // 判断是否可逆
  bool isInvertible() const;

  // 转换为D2D1::Matrix3x2F（用于Direct2D渲染）
  D2D1::Matrix3x2F toD2DMatrix() const;

  // 创建平移矩阵
  static Matrix33 translation(float x, float y);

  // 创建缩放矩阵
  static Matrix33 scaling(float x, float y, const Point &center = Point());

  // 创建旋转矩阵（角度为度）
  static Matrix33 rotation(float angle, const Point &center = Point());

  // 创建倾斜矩阵（角度为度）
  static Matrix33 skewing(float angle_x, float angle_y,
                          const Point &center = Point());

  // 计算逆矩阵
  static Matrix33 invert(const Matrix33 &matrix);
};

// 保持向后兼容的别名
using Matrix32 = Matrix33;
} // namespace easy2d
