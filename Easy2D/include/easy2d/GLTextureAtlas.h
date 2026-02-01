#pragma once
#include <easy2d/GLTexture.h>
#include <easy2d/e2dmacros.h>
#include <easy2d/e2dmath.h>
#include <glad/glad.h>
#include <map>
#include <memory>
#include <vector>

namespace easy2d {

/**
 * @brief 纹理图集区域信息
 *
 * 存储纹理在图集中的位置和UV坐标
 */
struct AtlasRegion {
  GLuint textureID;         // 纹理ID（用于查找）
  GLuint originalTextureID; // 原始纹理ID（用于查找）
  Rect region;              // 在图集中的矩形区域（像素坐标）
  Point uv0;                // 左上角UV坐标
  Point uv1;                // 右下角UV坐标
  int pageIndex;            // 所在图集页面索引

  AtlasRegion()
      : textureID(0), originalTextureID(0), region(), uv0(0.0f, 0.0f), uv1(1.0f, 1.0f),
        pageIndex(0) {}
};

/**
 * @brief 纹理图集页面
 *
 * 单个大纹理，包含多个小纹理
 */
class GLTextureAtlasPage {
public:
  /**
   * @brief 默认构造函数
   */
  GLTextureAtlasPage();

  /**
   * @brief 构造函数
   * @param width 图集宽度
   * @param height 图集高度
   */
  GLTextureAtlasPage(int width, int height);

  /**
   * @brief 析构函数
   */
  ~GLTextureAtlasPage();

  /**
   * @brief 初始化图集页面
   * @return 是否初始化成功
   */
  bool initialize();

  /**
   * @brief 尝试将纹理打包到图集中
   * @param texture 要打包的纹理
   * @param outRegion 输出的区域信息
   * @return 是否成功打包
   */
  bool packTexture(GLTexture *texture, AtlasRegion &outRegion);

  /**
   * @brief 获取OpenGL纹理ID
   */
  GLuint getTextureID() const { return _textureID; }

  /**
   * @brief 获取图集纹理
   */
  GLTexture *getTexture() const { return _texture; }

  /**
   * @brief 获取图集宽度
   */
  int getWidth() const { return _width; }

  /**
   * @brief 获取图集高度
   */
  int getHeight() const { return _height; }

  /**
   * @brief 获取已使用面积比例
   */
  float getUsageRatio() const;

  /**
   * @brief 绑定纹理
   * @param slot 纹理槽位
   */
  void bind(int slot = 0) const;

  /**
   * @brief 检查页面是否已初始化
   */
  bool isInitialized() const { return _initialized; }

private:
  /**
   * @brief 查找合适的空闲区域
   * @param width 需要宽度
   * @param height 需要高度
   * @param outRect 输出的矩形区域
   * @return 是否找到合适区域
   */
  bool findFreeRegion(int width, int height, Rect &outRect);

  /**
   * @brief 标记区域为已使用
   * @param rect 区域矩形
   */
  void markRegionUsed(const Rect &rect);

  /**
   * @brief 复制纹理数据到图集
   * @param texture 源纹理
   * @param destRect 目标区域
   */
  void copyTextureData(GLTexture *texture, const Rect &destRect);

  /**
   * @brief 清理资源
   */
  void cleanup();

private:
  int _width = 0;                // 图集宽度
  int _height = 0;               // 图集高度
  GLuint _textureID = 0;         // OpenGL纹理ID
  GLTexture *_texture = nullptr; // 纹理对象
  bool _initialized = false;     // 是否已初始化

  // 使用简单的网格系统跟踪已使用区域
  std::vector<bool> _usedGrid;
  int _gridCellSize = 16; // 网格单元大小（像素）
  int _gridWidth = 0;     // 网格宽度
  int _gridHeight = 0;    // 网格高度

  // 已占用的区域列表
  std::vector<Rect> _occupiedRegions;
};

/**
 * @brief 纹理图集管理器
 *
 * 管理多个图集页面，自动将纹理打包到图集中
 */
class GLTextureAtlas {
public:
  /**
   * @brief 获取单例实例（线程安全）
   */
  static GLTextureAtlas &getInstance();

  /**
   * @brief 初始化纹理图集系统
   * @param pageWidth 图集页面宽度（默认2048）
   * @param pageHeight 图集页面高度（默认2048）
   * @param maxPages 最大页面数（默认4）
   * @return 是否初始化成功
   */
  bool initialize(int pageWidth = 2048, int pageHeight = 2048,
                  int maxPages = 4);

  /**
   * @brief 关闭纹理图集系统
   */
  void shutdown();

  /**
   * @brief 检查是否已初始化
   */
  bool isInitialized() const { return _initialized; }

  /**
   * @brief 将纹理添加到图集
   * @param texture 要添加的纹理
   * @return 区域信息指针，如果失败返回nullptr
   */
  const AtlasRegion *addTexture(GLTexture *texture);

  /**
   * @brief 从图集中移除纹理
   * @param texture 要移除的纹理
   * @return 是否成功移除
   */
  bool removeTexture(GLTexture *texture);

  /**
   * @brief 获取纹理在图集中的区域信息
   * @param texture 纹理对象
   * @return 区域信息指针，如果不在图集中返回nullptr
   */
  const AtlasRegion *getRegion(GLTexture *texture) const;

  /**
   * @brief 检查纹理是否在图集中
   * @param texture 纹理对象
   * @return 是否在图集中
   */
  bool containsTexture(GLTexture *texture) const;

  /**
   * @brief 获取图集页面数量
   */
  int getPageCount() const;

  /**
   * @brief 获取指定页面
   * @param index 页面索引
   * @return 页面指针，如果索引无效返回nullptr
   */
  GLTextureAtlasPage *getPage(int index) const;

  /**
   * @brief 清空所有图集
   */
  void clear();

  /**
   * @brief 获取统计信息
   * @param outTotalPages 总页面数
   * @param outTotalTextures 总纹理数
   * @param outAverageUsage 平均使用率
   */
  void getStats(int &outTotalPages, int &outTotalTextures,
                float &outAverageUsage) const;

private:
  GLTextureAtlas() = default;
  ~GLTextureAtlas() = default;
  GLTextureAtlas(const GLTextureAtlas &) = delete;
  GLTextureAtlas &operator=(const GLTextureAtlas &) = delete;

  /**
   * @brief 创建新的图集页面
   * @return 新页面指针，如果达到最大页面数限制返回nullptr
   */
  GLTextureAtlasPage *createNewPage();

  /**
   * @brief 判断纹理是否适合放入图集
   * @param texture 纹理对象
   * @return 是否适合
   */
  bool isTextureSuitable(GLTexture *texture) const;

  /**
   * @brief 获取纹理的唯一键值
   * @param texture 纹理对象
   * @return 纹理ID作为键值
   */
  GLuint getTextureKey(GLTexture *texture) const;

private:
  int _pageWidth = 2048;     // 图集页面宽度
  int _pageHeight = 2048;    // 图集页面高度
  int _maxPages = 4;         // 最大页面数
  int _padding = 2;          // 纹理间填充像素（避免边缘采样问题）
  int _maxTextureSize = 512; // 最大可打包纹理尺寸

  std::vector<std::unique_ptr<GLTextureAtlasPage>> _pages;
  std::map<GLuint, AtlasRegion> _textureRegions;
  bool _initialized = false;
};

// 便捷宏定义
#define E2D_GL_TEXTURE_ATLAS easy2d::GLTextureAtlas::getInstance()

} // namespace easy2d
