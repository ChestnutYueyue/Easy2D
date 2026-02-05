#include <easy2d/math/e2dmath.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

// 默认构造函数，初始化为单位矩阵
easy2d::Matrix33::Matrix33() : matrix(1.0f) {}

// 从glm::mat4构造
easy2d::Matrix33::Matrix33(const glm::mat4 &m) : matrix(m) {}

// 从元素构造（2D仿射变换的6个参数）
// 参数对应2D仿射变换矩阵的6个元素（逻辑行主序）：
// [m00  m01  m02]
// [m10  m11  m12]
// [0    0    1  ]
//
// 在glm列主序4x4矩阵中存储为：
// 第0列: m00, m10, 0, 0
// 第1列: m01, m11, 0, 0
// 第2列: 0,   0,   1, 0
// 第3列: m02, m12, 0, 1  <- 平移值在第3列！
easy2d::Matrix33::Matrix33(float m00, float m01, float m02, float m10,
                           float m11, float m12)
    : matrix(1.0f) {
  // 设置2D仿射变换矩阵（4x4中的相关部分）
  matrix[0][0] = m00; matrix[0][1] = m10; matrix[0][2] = 0.0f; matrix[0][3] = 0.0f;
  matrix[1][0] = m01; matrix[1][1] = m11; matrix[1][2] = 0.0f; matrix[1][3] = 0.0f;
  matrix[2][0] = 0.0f; matrix[2][1] = 0.0f; matrix[2][2] = 1.0f; matrix[2][3] = 0.0f;
  matrix[3][0] = m02; matrix[3][1] = m12; matrix[3][2] = 0.0f; matrix[3][3] = 1.0f;
}

// 拷贝构造函数
easy2d::Matrix33::Matrix33(const Matrix33 &other) : matrix(other.matrix) {}

// 赋值运算符
easy2d::Matrix33 &easy2d::Matrix33::operator=(const Matrix33 &other) {
  matrix = other.matrix;
  return *this;
}

// 矩阵乘法
easy2d::Matrix33 easy2d::Matrix33::operator*(const Matrix33 &other) const {
  return Matrix33(matrix * other.matrix);
}

easy2d::Matrix33 &easy2d::Matrix33::operator*=(const Matrix33 &other) {
  matrix *= other.matrix;
  return *this;
}

// 下标访问 - 非const版本
// 提供行主序访问接口：operator()(row, col)
// 注意：对于2D仿射变换，平移值在第3列（索引3）
float &easy2d::Matrix33::operator()(int row, int col) {
  // 将逻辑上的2D仿射变换列索引映射到4x4矩阵的实际列索引
  // 2D的列0,1保持为4x4的列0,1
  // 2D的列2（平移）映射到4x4的列3
  int actualCol = (col == 2) ? 3 : col;
  return matrix[actualCol][row];
}

// 下标访问 - const版本
float easy2d::Matrix33::operator()(int row, int col) const {
  int actualCol = (col == 2) ? 3 : col;
  return matrix[actualCol][row];
}

// 设置为单位矩阵
void easy2d::Matrix33::identity() { matrix = glm::mat4(1.0f); }

// 变换向量
easy2d::Vector2 easy2d::Matrix33::transform(const Vector2 &v) const {
  glm::vec4 vec(v.x, v.y, 0.0f, 1.0f); // 2D点用齐次坐标表示，z=0
  glm::vec4 result = matrix * vec;
  return Vector2(result.x, result.y);
}

// 变换矩形
easy2d::Rect easy2d::Matrix33::transform(const Rect &rect) const {
  Vector2 top_left = transform(rect.getLeftTop());
  Vector2 top_right = transform(rect.getRightTop());
  Vector2 bottom_left = transform(rect.getLeftBottom());
  Vector2 bottom_right = transform(rect.getRightBottom());

  float left = std::min(std::min(top_left.x, top_right.x),
                        std::min(bottom_left.x, bottom_right.x));
  float right = std::max(std::max(top_left.x, top_right.x),
                         std::max(bottom_left.x, bottom_right.x));
  float top = std::min(std::min(top_left.y, top_right.y),
                       std::min(bottom_left.y, bottom_right.y));
  float bottom = std::max(std::max(top_left.y, top_right.y),
                          std::max(bottom_left.y, bottom_right.y));

  return Rect{Point{left, top}, Point{right, bottom}};
}

// 平移
void easy2d::Matrix33::translate(float x, float y) {
  // 使用glm::translate自动计算平移矩阵
  matrix = glm::translate(matrix, glm::vec3(x, y, 0.0f));
}

void easy2d::Matrix33::translate(const Vector2 &v) { translate(v.x, v.y); }

// 计算行列式
float easy2d::Matrix33::determinant() const {
  return glm::determinant(matrix);
}

// 判断是否为单位矩阵
bool easy2d::Matrix33::isIdentity() const {
  return matrix == glm::mat4(1.0f);
}

// 判断是否可逆
bool easy2d::Matrix33::isInvertible() const { return determinant() != 0.0f; }

// 转换为D2D1::Matrix3x2F（用于Direct2D渲染）
D2D1::Matrix3x2F easy2d::Matrix33::toD2DMatrix() const {
  // 从4x4矩阵提取2x3仿射变换矩阵
  // glm::mat4 存储为列主序，访问时是 mat[col][row]
  // D2D1::Matrix3x2F 是行主序: _11, _12, _21, _22, _31, _32
  // 
  // 对于2D仿射变换，我们使用4x4矩阵的左上3x3部分：
  // [m00  m01  m02]    [matrix[0][0]  matrix[1][0]  matrix[2][0]]
  // [m10  m11  m12] =  [matrix[0][1]  matrix[1][1]  matrix[2][1]]
  // [0    0    1  ]    [0             0             1           ]
  //
  // 注意：平移值在4x4矩阵的第3列（索引3），不是第2列！
  // 平移向量是 [matrix[3][0], matrix[3][1]]
  //
  // D2D1::Matrix3x2F 布局：
  // [_11  _12]   [m00  m01]
  // [_21  _22] = [m10  m11]
  // [_31  _32]   [m02  m12]  (平移部分)
  return D2D1::Matrix3x2F(
      matrix[0][0], matrix[1][0], // 第一行: _11, _12 -> m00, m01
      matrix[0][1], matrix[1][1], // 第二行: _21, _22 -> m10, m11
      matrix[3][0], matrix[3][1]  // 第三行: _31, _32 -> m02, m12 (平移) - 使用第3列！
  );
}

// 创建平移矩阵
easy2d::Matrix33 easy2d::Matrix33::translation(float x, float y) {
  Matrix33 result;
  // 使用glm::translate自动计算
  result.matrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
  return result;
}

// 创建缩放矩阵
easy2d::Matrix33 easy2d::Matrix33::scaling(float x, float y,
                                           const Point &center) {
  Matrix33 result;
  glm::mat4 m(1.0f);
  // 使用glm函数自动计算: 先平移到原点，缩放，再平移回去
  m = glm::translate(m, glm::vec3(center.x, center.y, 0.0f));
  m = glm::scale(m, glm::vec3(x, y, 1.0f));
  m = glm::translate(m, glm::vec3(-center.x, -center.y, 0.0f));
  result.matrix = m;
  return result;
}

// 创建旋转矩阵（角度为度）
easy2d::Matrix33 easy2d::Matrix33::rotation(float angle,
                                            const Point &center) {
  Matrix33 result;
  // 将角度转换为弧度
  float radians = angle * math::constants::DEG_TO_RAD_F;
  glm::mat4 m(1.0f);
  // 使用glm::rotate自动计算: 先平移到原点，绕Z轴旋转，再平移回去
  m = glm::translate(m, glm::vec3(center.x, center.y, 0.0f));
  m = glm::rotate(m, radians, glm::vec3(0.0f, 0.0f, 1.0f)); // 绕Z轴旋转
  m = glm::translate(m, glm::vec3(-center.x, -center.y, 0.0f));
  result.matrix = m;
  return result;
}

// 创建倾斜矩阵（角度为度）
easy2d::Matrix33 easy2d::Matrix33::skewing(float angle_x, float angle_y,
                                           const Point &center) {
  Matrix33 result;
  // 将角度转换为弧度
  float rad_x = angle_x * math::constants::DEG_TO_RAD_F;
  float rad_y = angle_y * math::constants::DEG_TO_RAD_F;
  float tx = std::tan(rad_x);
  float ty = std::tan(rad_y);

  // 构建倾斜矩阵（手动构建，因为glm没有直接的skew函数）
  // 先平移到原点，倾斜，再平移回去
  glm::mat4 m(1.0f);
  m = glm::translate(m, glm::vec3(center.x, center.y, 0.0f));

  // 应用倾斜变换
  // 2D倾斜矩阵（逻辑行主序）：
  // [1    ty   0]
  // [tx   1    0]
  // [0    0    1]
  // 
  // 在glm列主序中：
  // 第0列: 1, tx, 0, 0
  // 第1列: ty, 1, 0, 0
  // 第2列: 0, 0, 1, 0
  glm::mat4 skew(1.0f);
  skew[0][1] = tx; // 第0列第1行 = tx
  skew[1][0] = ty; // 第1列第0行 = ty
  m = m * skew;

  m = glm::translate(m, glm::vec3(-center.x, -center.y, 0.0f));
  result.matrix = m;
  return result;
}

// 计算逆矩阵
easy2d::Matrix33 easy2d::Matrix33::invert(const Matrix33 &matrix) {
  Matrix33 result;
  result.matrix = glm::inverse(matrix.matrix);
  return result;
}
