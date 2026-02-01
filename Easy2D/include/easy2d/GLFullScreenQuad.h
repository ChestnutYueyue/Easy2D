#pragma once
#include <easy2d/e2dmacros.h>
#include <glad/glad.h>

namespace easy2d
{

/**
 * @brief 全屏四边形渲染器
 * 
 * 用于后期处理效果的全屏四边形渲染
 */
class GLFullScreenQuad
{
public:
    /**
     * @brief 获取单例实例
     */
    static GLFullScreenQuad& getInstance();

    /**
     * @brief 初始化
     * @return 是否初始化成功
     */
    bool initialize();

    /**
     * @brief 关闭
     */
    void shutdown();

    /**
     * @brief 渲染全屏四边形
     */
    void render() const;

    /**
     * @brief 获取VAO
     * @return VAO ID
     */
    GLuint getVAO() const { return _vao; }

private:
    GLFullScreenQuad() = default;
    ~GLFullScreenQuad() = default;
    GLFullScreenQuad(const GLFullScreenQuad&) = delete;
    GLFullScreenQuad& operator=(const GLFullScreenQuad&) = delete;

private:
    GLuint _vao = 0;
    GLuint _vbo = 0;
    bool _initialized = false;
};

// 便捷宏定义
#define E2D_GL_FULLSCREEN_QUAD easy2d::GLFullScreenQuad::getInstance()

} // namespace easy2d
