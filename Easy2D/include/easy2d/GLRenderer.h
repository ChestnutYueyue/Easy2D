#pragma once
#include <easy2d/e2dmacros.h>
#include <glad/glad.h>
#include <easy2d/e2dmath.h>
#include <easy2d/e2dcommon.h>
#include "GLShader.h"
#include "GLTexture.h"
#include "GLFrameBuffer.h"
#include "GLBuffer.h"

// 前向声明SDL类型
struct SDL_Window;

typedef void* SDL_GLContext;

namespace easy2d
{

// 前向声明
class Node;
class TextLayout;
class Shape;
class Text;

/**
 * @brief OpenGL渲染器
 *
 * 替代D2D1的Renderer类，提供完整的OpenGL渲染功能
 */
class GLRenderer
{
public:
    /**
     * @brief 获取单例实例
     */
    static GLRenderer& getInstance();

    /**
     * @brief 初始化渲染器
     * @param window SDL窗口指针
     * @param width 窗口宽度
     * @param height 窗口高度
     * @return 是否初始化成功
     */
    bool initialize(SDL_Window* window, int width, int height);

    /**
     * @brief 关闭渲染器
     */
    void shutdown();

    /**
     * @brief 调整渲染器大小
     * @param width 新宽度
     * @param height 新高度
     */
    void resize(int width, int height);

    /**
     * @brief 开始一帧渲染
     */
    void beginFrame();

    /**
     * @brief 结束一帧渲染并交换缓冲区
     */
    void endFrame();

    /**
     * @brief 清空屏幕
     * @param color 清除颜色
     */
    void clear(const Color& color);

    /**
     * @brief 设置背景色
     * @param color 背景颜色
     */
    void setBackgroundColor(const Color& color);

    /**
     * @brief 获取背景色
     */
    Color getBackgroundColor() const;

    /**
     * @brief 设置投影矩阵（正交投影）
     * @param left 左边界
     * @param right 右边界
     * @param bottom 下边界
     * @param top 上边界
     */
    void setProjection(float left, float right, float bottom, float top);

    /**
     * @brief 设置模型矩阵
     * @param matrix 变换矩阵
     */
    void setModelMatrix(const Matrix32& matrix);

    /**
     * @brief 设置不透明度
     * @param opacity 不透明度（0.0 - 1.0）
     */
    void setOpacity(float opacity);

    /**
     * @brief 绘制纹理
     * @param texture 纹理对象
     * @param destRect 目标矩形
     * @param srcRect 源矩形（可选，nullptr表示整个纹理）
     * @param color 颜色叠加（可选）
     */
    void drawTexture(GLTexture* texture, const Rect& destRect, const Rect* srcRect = nullptr, const Color& color = Color::White);

    /**
     * @brief 绘制矩形（填充）
     * @param rect 矩形
     * @param color 颜色
     */
    void drawFilledRect(const Rect& rect, const Color& color);

    /**
     * @brief 绘制矩形（填充或描边）
     * @param rect 矩形
     * @param color 颜色
     */
    void drawRect(const Rect& rect, const Color& color);

    /**
     * @brief 绘制矩形（描边）
     * @param rect 矩形
     * @param color 颜色
     * @param lineWidth 线宽
     */
    void drawRectOutline(const Rect& rect, const Color& color, float lineWidth = 1.0f);

    /**
     * @brief 绘制圆角矩形（填充）
     * @param rect 矩形
     * @param radius 圆角半径
     * @param color 颜色
     */
    void drawFilledRoundRect(const Rect& rect, const Vector2& radius, const Color& color);

    /**
     * @brief 绘制圆角矩形（描边）
     * @param rect 矩形
     * @param radius 圆角半径
     * @param color 颜色
     * @param lineWidth 线宽
     */
    void drawRoundRectOutline(const Rect& rect, const Vector2& radius, const Color& color, float lineWidth = 1.0f);

    /**
     * @brief 绘制椭圆（填充）
     * @param center 中心点
     * @param radius 半径
     * @param color 颜色
     */
    void drawFilledEllipse(const Point& center, const Vector2& radius, const Color& color);

    /**
     * @brief 绘制椭圆（描边）
     * @param center 中心点
     * @param radius 半径
     * @param color 颜色
     * @param lineWidth 线宽
     */
    void drawEllipseOutline(const Point& center, const Vector2& radius, const Color& color, float lineWidth = 1.0f);

    /**
     * @brief 绘制圆形（填充）
     * @param center 中心点
     * @param radius 半径
     * @param color 颜色
     */
    void drawFilledCircle(const Point& center, float radius, const Color& color);

    /**
     * @brief 绘制圆形（描边）
     * @param center 中心点
     * @param radius 半径
     * @param color 颜色
     * @param lineWidth 线宽
     */
    void drawCircleOutline(const Point& center, float radius, const Color& color, float lineWidth = 1.0f);

    /**
     * @brief 绘制线条
     * @param start 起点
     * @param end 终点
     * @param color 颜色
     * @param lineWidth 线宽
     */
    void drawLine(const Point& start, const Point& end, const Color& color, float lineWidth = 1.0f);

    /**
     * @brief 绘制多边形（填充）
     * @param points 顶点数组
     * @param count 顶点数量
     * @param color 颜色
     */
    void drawFilledPolygon(const Point* points, int count, const Color& color);

    /**
     * @brief 绘制多边形（描边）
     * @param points 顶点数组
     * @param count 顶点数量
     * @param color 颜色
     * @param lineWidth 线宽
     */
    void drawPolygonOutline(const Point* points, int count, const Color& color, float lineWidth = 1.0f);

    /**
     * @brief 绘制文本布局
     * @param layout 文本布局
     * @param pos 位置
     * @param style 绘图样式
     */
    void drawTextLayout(TextLayout* layout, const Point& pos, const DrawingStyle& style);

    /**
     * @brief 渲染场景
     * @param showBodyShapes 是否显示物理形状
     */
    void renderScene(bool showBodyShapes = false);

    /**
     * @brief 显示FPS
     * @param show 是否显示
     */
    void showFps(bool show);

    /**
     * @brief 显示物理形状
     * @param show 是否显示
     */
    void showBodyShapes(bool show);

    /**
     * @brief 设置垂直同步
     * @param enabled 是否启用
     */
    void setVSync(bool enabled);

    /**
     * @brief 是否启用垂直同步
     */
    bool isVSyncEnabled() const { return _vSyncEnabled; }

    /**
     * @brief 获取窗口宽度
     */
    int getWindowWidth() const { return _windowWidth; }

    /**
     * @brief 获取窗口高度
     */
    int getWindowHeight() const { return _windowHeight; }

    /**
     * @brief 获取DPI缩放X
     */
    float getDpiScaleX() const { return _dpiScaleX; }

    /**
     * @brief 获取DPI缩放Y
     */
    float getDpiScaleY() const { return _dpiScaleY; }

    /**
     * @brief 获取着色器管理器
     */
    GLShaderManager* getShaderManager() const { return &_shaderManager; }

    /**
     * @brief 获取纹理缓存
     */
    GLTextureCache* getTextureCache() const { return &_textureCache; }

    /**
     * @brief 获取渲染目标管理器
     */
    GLRenderTarget* getRenderTarget() { return &_renderTarget; }

    /**
     * @brief 设备资源是否刚刚重建
     */
    bool isDeviceResourceRecreated() const { return _deviceResourceRecreated; }

private:
    GLRenderer() = default;
    ~GLRenderer() = default;
    GLRenderer(const GLRenderer&) = delete;
    GLRenderer& operator=(const GLRenderer&) = delete;

    /**
     * @brief 初始化OpenGL上下文
     */
    bool initializeGLContext(SDL_Window* window);

    /**
     * @brief 销毁OpenGL上下文
     */
    void destroyGLContext();

    /**
     * @brief 初始化默认资源
     */
    bool initializeResources();

    /**
     * @brief 渲染FPS显示
     */
    void renderFps();

    /**
     * @brief 创建设备相关资源
     */
    bool createDeviceResources();

    /**
     * @brief 销毁设备相关资源
     */
    void discardDeviceResources();

private:
    // SDL相关
    SDL_Window* _window = nullptr;           // SDL窗口指针
    SDL_GLContext _glContext = nullptr;      // OpenGL渲染上下文

    // 窗口尺寸
    int _windowWidth = 0;
    int _windowHeight = 0;

    // DPI缩放
    float _dpiScaleX = 96.0f;
    float _dpiScaleY = 96.0f;

    // 状态
    bool _initialized = false;
    bool _vSyncEnabled = true;
    bool _showFps = false;
    bool _showBodyShapes = false;
    bool _deviceResourceRecreated = false;

    // 颜色
    Color _backgroundColor = Color::Black;
    Color _clearColor = Color::Black;

    // 矩阵
    float _projectionMatrix[16];
    Matrix32 _modelMatrix;
    float _opacity = 1.0f;

    // 子系统
    GLShaderManager& _shaderManager = GLShaderManager::getInstance();
    GLTextureCache& _textureCache = GLTextureCache::getInstance();
    GLRenderTarget& _renderTarget = GLRenderTarget::getInstance();

    // 批次渲染器
    GLBatchRenderer* _batchRenderer = nullptr;

    // 默认纹理（白色像素，用于纯色绘制）
    GLTexture* _defaultTexture = nullptr;

    // FPS统计
    int _fpsFrameCount = 0;
    float _fpsLastTime = 0.0f;
    float _fpsCurrent = 0.0f;

    // FPS显示文本
    Text* _fpsText = nullptr;
};

// 便捷宏定义
#define E2D_GL_RENDERER easy2d::GLRenderer::getInstance()

} // namespace easy2d
