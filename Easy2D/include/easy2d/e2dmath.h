#pragma once
#include <easy2d/e2dmacros.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>

namespace easy2d
{
	namespace math
	{
		namespace constants
		{
			// 使用 GLM 的常量
			constexpr float PI_F = glm::pi<float>();
			constexpr float PI_F_2 = glm::half_pi<float>();
			constexpr float PI_F_X_2 = glm::two_pi<float>();

			constexpr double PI_D = glm::pi<double>();
			constexpr double PI_D_2 = glm::half_pi<double>();
			constexpr double PI_D_X_2 = glm::two_pi<double>();
		}

		// 使用 GLM 的数学函数
		template<typename T>
		inline T Abs(T val) { return glm::abs(val); }

		inline float Sqrt(float val) { return glm::sqrt(val); }
		inline double Sqrt(double val) { return glm::sqrt(val); }

		inline float Pow(float base, float exponent) { return glm::pow(base, exponent); }
		inline double Pow(double base, double exponent) { return glm::pow(base, exponent); }

		template<typename T>
		inline T Sign(T val) { return glm::sign(val); }

		// 角度制转弧度制后计算三角函数
		inline float Sin(float val) { return glm::sin(val * constants::PI_F / 180.f); }
		inline double Sin(double val) { return glm::sin(val * constants::PI_D / 180.0); }

		inline float Cos(float val) { return glm::cos(val * constants::PI_F / 180.f); }
		inline double Cos(double val) { return glm::cos(val * constants::PI_D / 180.0); }

		inline float Tan(float val) { return glm::tan(val * constants::PI_F / 180.f); }
		inline double Tan(double val) { return glm::tan(val * constants::PI_D / 180.0); }

		// 反三角函数，结果转角度制
		inline float Asin(float val) { return glm::degrees(glm::asin(val)); }
		inline double Asin(double val) { return glm::degrees(glm::asin(val)); }

		inline float Acos(float val) { return glm::degrees(glm::acos(val)); }
		inline double Acos(double val) { return glm::degrees(glm::acos(val)); }

		inline float Atan(float val) { return glm::degrees(glm::atan(val)); }
		inline double Atan(double val) { return glm::degrees(glm::atan(val)); }

		inline float Atan2(float y, float x) { return glm::degrees(glm::atan(y, x)); }
		inline double Atan2(double y, double x) { return glm::degrees(glm::atan(y, x)); }

		inline float Ceil(float val) { return glm::ceil(val); }
		inline double Ceil(double val) { return glm::ceil(val); }

		inline float Floor(float val) { return glm::floor(val); }
		inline double Floor(double val) { return glm::floor(val); }

		inline float Round(float val) { return glm::round(val); }
		inline double Round(double val) { return glm::round(val); }

		inline float Min(float a, float b) { return glm::min(a, b); }
		inline float Max(float a, float b) { return glm::max(a, b); }
		inline double Min(double a, double b) { return glm::min(a, b); }
		inline double Max(double a, double b) { return glm::max(a, b); }

		inline float Clamp(float val, float minVal, float maxVal) { return glm::clamp(val, minVal, maxVal); }
		inline double Clamp(double val, double minVal, double maxVal) { return glm::clamp(val, minVal, maxVal); }

		// 线性插值
		inline float Lerp(float a, float b, float t) { return glm::mix(a, b, t); }
		inline double Lerp(double a, double b, double t) { return glm::mix(a, b, t); }
	}

	class Size;

	// 坐标 - 使用 GLM vec2 作为底层存储
	class Point
	{
	public:
		float x;	// X 坐标
		float y;	// Y 坐标

	public:
		Point() : x(0), y(0) {}
		Point(float x, float y) : x(x), y(y) {}
		Point(const Point& other) : x(other.x), y(other.y) {}
		Point(const glm::vec2& v) : x(v.x), y(v.y) {}

		Point operator + (Point const& point) const { return Point(x + point.x, y + point.y); }
		Point operator - (Point const& point) const { return Point(x - point.x, y - point.y); }
		Point operator * (float const& point) const { return Point(x * point, y * point); }
		Point operator / (float const& point) const { return Point(x / point, y / point); }
		Point operator - () const { return Point(-x, -y); }
		bool operator== (const Point& point) const { return x == point.x && y == point.y; }
		bool operator!= (const Point& point) const { return !operator==(point); }

		operator easy2d::Size() const;
		operator glm::vec2() const { return glm::vec2(x, y); }

		bool isEmpty() const { return !x && !y; }

		// 转换为 GLM vec2
		glm::vec2 toVec2() const { return glm::vec2(x, y); }

		// 判断两点间距离
		static float distance(const Point& p1, const Point& p2)
		{
			return glm::distance(glm::vec2(p1.x, p1.y), glm::vec2(p2.x, p2.y));
		}
	};


	// 二维向量
	using Vector2 = Point;

	// 大小
	class Size
	{
	public:
		float width;	// 宽度
		float height;	// 高度

	public:
		Size() : width(0), height(0) {}
		Size(float width, float height) : width(width), height(height) {}
		Size(const Size& other) : width(other.width), height(other.height) {}

		Size operator + (Size const& size) const { return Size(width + size.width, height + size.height); }
		Size operator - (Size const& size) const { return Size(width - size.width, height - size.height); }
		Size operator * (float const& size) const { return Size(width * size, height * size); }
		Size operator / (float const& size) const { return Size(width / size, height / size); }
		Size operator - () const { return Size(-width, -height); }
		bool operator== (const Size& size) const { return width == size.width && height == size.height; }
		bool operator!= (const Size& size) const { return !operator==(size); }

		operator easy2d::Point() const { return Point(width, height); }
		operator glm::vec2() const { return glm::vec2(width, height); }

		bool isEmpty() const { return !width && !height; }
	};


	// 矩形
	class Rect
	{
	public:
		Point leftTop, rightBottom;

	public:
		Rect() = default;
		Rect(const Point& lt, const Point& rb) : leftTop(lt), rightBottom(rb) {}
		Rect(const Point& pos, const Size& size) { setRect(pos, size); }
		Rect(const Rect& other) : leftTop(other.leftTop), rightBottom(other.rightBottom) {}

		Rect& operator= (const Rect& other)
		{
			leftTop = other.leftTop;
			rightBottom = other.rightBottom;
			return *this;
		}

		bool operator== (const Rect& rect) const { return leftTop == rect.leftTop && rightBottom == rect.rightBottom; }

		bool isEmpty() const { return leftTop.isEmpty() && rightBottom.isEmpty(); }

		void setRect(const Point& pos, const Size& size)
		{
			leftTop = pos;
			rightBottom = Point{ pos.x + size.width, pos.y + size.height };
		}

		bool containsPoint(const Point& point) const
		{
			return point.x >= leftTop.x && point.x <= rightBottom.x && point.y >= leftTop.y && point.y <= rightBottom.y;
		}

		bool intersects(const Rect& rect) const
		{
			return !(rightBottom.x < rect.leftTop.x || rect.rightBottom.x < leftTop.x ||
					rightBottom.y < rect.leftTop.y || rect.rightBottom.y < leftTop.y);
		}

		inline float getWidth() const { return rightBottom.x - leftTop.x; }
		inline float getHeight() const { return rightBottom.y - leftTop.y; }
		inline Size getSize() const { return Size{ getWidth(), getHeight() }; }
		inline Vector2 getLeftTop() const { return leftTop; }
		inline Vector2 getRightBottom() const { return rightBottom; }
		inline Vector2 getRightTop() const { return Vector2{ getRight(), getTop() }; }
		inline Vector2 getLeftBottom() const { return Vector2{ getLeft(), getBottom() }; }
		inline float getLeft() const { return leftTop.x; }
		inline float getTop() const { return leftTop.y; }
		inline float getRight() const { return rightBottom.x; }
		inline float getBottom() const { return rightBottom.y; }
	};


	// 二维变换矩阵 - 使用 GLM mat3 作为底层存储
	struct Matrix32
	{
		union
		{
			struct
			{
				float m[6];  // m[3][2]
			};

			struct
			{
				float
					_11, _12,
					_21, _22,
					_31, _32;
			};
		};

		Matrix32()
			: _11(1.f), _12(0.f)
			, _21(0.f), _22(1.f)
			, _31(0.f), _32(0.f)
		{
		}

		Matrix32(float _11, float _12, float _21, float _22, float _31, float _32)
			: _11(_11), _12(_12), _21(_21), _22(_22), _31(_31), _32(_32)
		{
		}

		Matrix32(Matrix32 const& other)
			: _11(other._11), _12(other._12)
			, _21(other._21), _22(other._22)
			, _31(other._31), _32(other._32)
		{
		}

		// 从 GLM mat3 构造
		Matrix32(const glm::mat3& mat)
		{
			_11 = mat[0][0]; _12 = mat[0][1];
			_21 = mat[1][0]; _22 = mat[1][1];
			_31 = mat[2][0]; _32 = mat[2][1];
		}

		float operator [](unsigned int index) const { return m[index]; }

		void identity()
		{
			_11 = 1.f; _12 = 0.f;
			_21 = 0.f; _22 = 1.f;
			_31 = 0.f; _32 = 0.f;
		}

		Vector2 transform(const Vector2& v) const
		{
			return Vector2(
				v.x * _11 + v.y * _21 + _31,
				v.x * _12 + v.y * _22 + _32
			);
		}

		Point transformPoint(const Point& p) const
		{
			return Point(
				p.x * _11 + p.y * _21 + _31,
				p.x * _12 + p.y * _22 + _32
			);
		}

		Rect transform(const Rect& rect) const
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

		void translate(float x, float y)
		{
			_31 += _11 * x + _21 * y;
			_32 += _12 * x + _22 * y;
		}

		void translate(const Vector2& v)
		{
			translate(v.x, v.y);
		}

		float determinant() const
		{
			return (_11 * _22) - (_12 * _21);
		}

		bool isIdentity() const
		{
			return _11 == 1.f && _12 == 0.f &&
				_21 == 0.f && _22 == 1.f &&
				_31 == 0.f && _32 == 0.f;
		}

		bool isInvertible() const
		{
			return 0 != determinant();
		}

		// 转换为 GLM mat3
		glm::mat3 toMat3() const
		{
			return glm::mat3(
				_11, _12, 0.0f,
				_21, _22, 0.0f,
				_31, _32, 1.0f
			);
		}

		// 转换为 GLM mat4（用于 OpenGL 着色器）
		glm::mat4 toMat4() const
		{
			return glm::mat4(
				_11, _12, 0.0f, 0.0f,
				_21, _22, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				_31, _32, 0.0f, 1.0f
			);
		}

		// 转换为 D2D1 矩阵（保留兼容性）
		D2D1::Matrix3x2F const& toD2DMatrix() const
		{
			return reinterpret_cast<D2D1::Matrix3x2F const&>(*this);
		}

		static Matrix32 translation(float x, float y)
		{
			return Matrix32(
				1.f, 0.f,
				0.f, 1.f,
				x, y
			);
		}

		static Matrix32 scaling(float x, float y, const Point& center = Point())
		{
			return Matrix32(
				x, 0.f,
				0.f, y,
				center.x - x * center.x,
				center.y - y * center.y
			);
		}

		static Matrix32 rotation(float angle, const Point& center = Point())
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

		static Matrix32 skewing(float angle_x, float angle_y, const Point& center = Point())
		{
			float tx = glm::tan(angle_x * math::constants::PI_F / 180.f);
			float ty = glm::tan(angle_y * math::constants::PI_F / 180.f);
			return Matrix32(
				1.f, -ty,
				-tx, 1.f,
				center.y * tx, center.x * ty
			);
		}

		static Matrix32 invert(Matrix32 const& matrix)
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
		Matrix32 operator*(const Matrix32& other) const
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
	};
}
