#pragma once
#include <easy2d/e2dmacros.h>
#include <glad/glad.h>
#include "GLTexture.h"
#include <easy2d/e2dmath.h>

namespace easy2d
{

/**
 * @brief OpenGL帧缓冲对象类
 * 
 * 替代D2D1的ID2D1BitmapRenderTarget，用于离屏渲染
 */
class GLFrameBuffer
{
public:
    /**
     * @brief 构造函数
     */
    GLFrameBuffer();

    /**
     * @brief 析构函数，自动释放资源
     */
    ~GLFrameBuffer();

    /**
     * @brief 创建帧缓冲
     * @param width 宽度
     * @param height 高度
     * @param hasDepthStencil 是否创建深度/模板缓冲
     * @return 是否创建成功
     */
    bool create(int width, int height, bool hasDepthStencil = false);

    /**
     * @brief 调整大小
     * @param width 新宽度
     * @param height 新高度
     * @return 是否调整成功
     */
    bool resize(int width, int height);

    /**
     * @brief 绑定帧缓冲（开始离屏渲染）
     */
    void bind() const;

    /**
     * @brief 解绑帧缓冲（恢复默认帧缓冲）
     */
    void unbind() const;

    /**
     * @brief 清空帧缓冲
     * @param r 红色分量
     * @param g 绿色分量
     * @param b 蓝色分量
     * @param a 透明度分量
     */
    void clear(float r, float g, float b, float a = 1.0f) const;

    /**
     * @brief 获取颜色纹理
     * @return 颜色纹理对象
     */
    GLTexture* getColorTexture() const { return _colorTexture; }

    /**
     * @brief 获取帧缓冲ID
     * @return OpenGL帧缓冲ID
     */
    GLuint getFrameBufferID() const { return _frameBufferID; }

    /**
     * @brief 获取宽度
     */
    int getWidth() const { return _width; }

    /**
     * @brief 获取高度
     */
    int getHeight() const { return _height; }

    /**
     * @brief 获取大小
     */
    Size getSize() const { return Size(static_cast<float>(_width), static_cast<float>(_height)); }

    /**
     * @brief 检查帧缓冲是否完整
     */
    bool isComplete() const;

    /**
     * @brief 检查帧缓冲是否有效
     */
    bool isValid() const { return _frameBufferID != 0; }

private:
    /**
     * @brief 释放资源
     */
    void release();

    /**
     * @brief 检查帧缓冲状态
     */
    bool checkStatus() const;

private:
    GLuint _frameBufferID;      // 帧缓冲ID
    GLuint _depthStencilID;     // 深度/模板缓冲ID
    GLTexture* _colorTexture;   // 颜色纹理
    int _width;                 // 宽度
    int _height;                // 高度
    bool _hasDepthStencil;      // 是否有深度/模板缓冲
};

/**
 * @brief 渲染目标管理器
 * 
 * 管理主渲染目标和离屏渲染目标
 */
class GLRenderTarget
{
public:
    /**
     * @brief 获取单例实例
     */
    static GLRenderTarget& getInstance();

    /**
     * @brief 初始化主渲染目标
     * @param windowWidth 窗口宽度
     * @param windowHeight 窗口高度
     */
    void initialize(int windowWidth, int windowHeight);

    /**
     * @brief 调整主渲染目标大小
     * @param width 新宽度
     * @param height 新高度
     */
    void resize(int width, int height);

    /**
     * @brief 绑定主渲染目标
     */
    void bindMainTarget() const;

    /**
     * @brief 获取主渲染目标大小
     */
    Size getMainTargetSize() const { return Size(static_cast<float>(_windowWidth), static_cast<float>(_windowHeight)); }

    /**
     * @brief 获取窗口宽度
     */
    int getWindowWidth() const { return _windowWidth; }

    /**
     * @brief 获取窗口高度
     */
    int getWindowHeight() const { return _windowHeight; }

    /**
     * @brief 设置视口
     * @param x 视口X坐标
     * @param y 视口Y坐标
     * @param width 视口宽度
     * @param height 视口高度
     */
    void setViewport(int x, int y, int width, int height) const;

    /**
     * @brief 设置默认视口（全窗口）
     */
    void setDefaultViewport() const;

    /**
     * @brief 清空当前渲染目标
     * @param r 红色分量
     * @param g 绿色分量
     * @param b 蓝色分量
     * @param a 透明度分量
     */
    void clear(float r, float g, float b, float a = 1.0f) const;

    /**
     * @brief 设置清除颜色
     * @param r 红色分量
     * @param g 绿色分量
     * @param b 蓝色分量
     * @param a 透明度分量
     */
    void setClearColor(float r, float g, float b, float a = 1.0f);

private:
    GLRenderTarget() = default;
    ~GLRenderTarget() = default;
    GLRenderTarget(const GLRenderTarget&) = delete;
    GLRenderTarget& operator=(const GLRenderTarget&) = delete;

private:
    int _windowWidth = 0;   // 窗口宽度
    int _windowHeight = 0;  // 窗口高度
    float _clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };  // 清除颜色
};

// 便捷宏定义
#define E2D_GL_RENDER_TARGET easy2d::GLRenderTarget::getInstance()

} // namespace easy2d
