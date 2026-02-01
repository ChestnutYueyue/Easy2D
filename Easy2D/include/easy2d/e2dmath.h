#pragma once
#include <easy2d/e2dmacros.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/constants.hpp>

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
		Point();
		Point(float x, float y);
		Point(const Point& other);
		Point(const glm::vec2& v);

		Point operator + (Point const& point) const;
		Point operator - (Point const& point) const;
		Point operator * (float const& point) const;
		Point operator / (float const& point) const;
		Point operator - () const;
		bool operator== (const Point& point) const;
		bool operator!= (const Point& point) const;

		operator easy2d::Size() const;
		operator glm::vec2() const;

		bool isEmpty() const;

		// 转换为 GLM vec2
		glm::vec2 toVec2() const;

		// 判断两点间距离
		static float distance(const Point& p1, const Point& p2);
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
		Size();
		Size(float width, float height);
		Size(const Size& other);

		Size operator + (Size const& size) const;
		Size operator - (Size const& size) const;
		Size operator * (float const& size) const;
		Size operator / (float const& size) const;
		Size operator - () const;
		bool operator== (const Size& size) const;
		bool operator!= (const Size& size) const;

		operator easy2d::Point() const;
		operator glm::vec2() const;

		bool isEmpty() const;
	};


	// 矩形
	class Rect
	{
	public:
		Point leftTop, rightBottom;

	public:
		Rect();
		Rect(const Point& lt, const Point& rb);
		Rect(const Point& pos, const Size& size);
		Rect(const Rect& other);

		Rect& operator= (const Rect& other);

		bool operator== (const Rect& rect) const;

		bool isEmpty() const;

		void setRect(const Point& pos, const Size& size);

		bool containsPoint(const Point& point) const;

		bool intersects(const Rect& rect) const;

		float getWidth() const;
		float getHeight() const;
		Size getSize() const;
		Vector2 getLeftTop() const;
		Vector2 getRightBottom() const;
		Vector2 getRightTop() const;
		Vector2 getLeftBottom() const;
		float getLeft() const;
		float getTop() const;
		float getRight() const;
		float getBottom() const;
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

		Matrix32();
		Matrix32(float _11, float _12, float _21, float _22, float _31, float _32);
		Matrix32(Matrix32 const& other);
		Matrix32(const glm::mat3& mat);

		float operator [](unsigned int index) const;

		void identity();

		Vector2 transform(const Vector2& v) const;

		Point transformPoint(const Point& p) const;

		Rect transform(const Rect& rect) const;

		void translate(float x, float y);

		void translate(const Vector2& v);

		float determinant() const;

		bool isIdentity() const;

		bool isInvertible() const;

		// 转换为 GLM mat3
		glm::mat3 toMat3() const;

		// 转换为 GLM mat4（用于 OpenGL 着色器）
		glm::mat4 toMat4() const;

#ifndef E2D_NO_DIRECT2D
		// 转换为 D2D1 矩阵（保留兼容性）
		D2D1::Matrix3x2F const& toD2DMatrix() const;
#endif

		static Matrix32 translation(float x, float y);

		static Matrix32 scaling(float x, float y, const Point& center = Point());

		static Matrix32 rotation(float angle, const Point& center = Point());

		static Matrix32 skewing(float angle_x, float angle_y, const Point& center = Point());

		static Matrix32 invert(Matrix32 const& matrix);

		// 矩阵乘法
		Matrix32 operator*(const Matrix32& other) const;
	};
}
