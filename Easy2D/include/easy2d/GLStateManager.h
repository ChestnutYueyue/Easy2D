#pragma once
#include <easy2d/e2dmacros.h>
#include <glad/glad.h>
#include <easy2d/e2dmath.h>
#include <easy2d/e2dcommon.h>
#include <stack>

namespace easy2d
{

/**
 * @brief OpenGL渲染状态管理器
 * 
 * 管理OpenGL的各种渲染状态，避免冗余的状态切换
 */
class GLStateManager
{
public:
    /**
     * @brief 获取单例实例
     */
    static GLStateManager& getInstance();

    /**
     * @brief 初始化默认状态
     */
    void initialize();

    /**
     * @brief 重置所有状态
     */
    void reset();

    // 混合状态管理
    /**
     * @brief 设置混合模式
     * @param enabled 是否启用混合
     */
    void setBlend(bool enabled);

    /**
     * @brief 设置混合函数
     * @param srcFactor 源因子
     * @param dstFactor 目标因子
     */
    void setBlendFunc(GLenum srcFactor, GLenum dstFactor);

    /**
     * @brief 设置混合方程式
     * @param mode 混合模式
     */
    void setBlendEquation(GLenum mode);

    // 深度测试管理
    /**
     * @brief 设置深度测试
     * @param enabled 是否启用深度测试
     */
    void setDepthTest(bool enabled);

    /**
     * @brief 设置深度函数
     * @param func 深度测试函数
     */
    void setDepthFunc(GLenum func);

    /**
     * @brief 设置深度写入
     * @param enabled 是否启用深度写入
     */
    void setDepthWrite(bool enabled);

    // 模板测试管理
    /**
     * @brief 设置模板测试
     * @param enabled 是否启用模板测试
     */
    void setStencilTest(bool enabled);

    /**
     * @brief 设置模板函数
     * @param func 模板测试函数
     * @param ref 参考值
     * @param mask 掩码
     */
    void setStencilFunc(GLenum func, GLint ref, GLuint mask);

    /**
     * @brief 设置模板操作
     * @param sfail 模板测试失败操作
     * @param dpfail 深度测试失败操作
     * @param dppass 深度测试通过操作
     */
    void setStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass);

    // 面剔除管理
    /**
     * @brief 设置面剔除
     * @param enabled 是否启用面剔除
     */
    void setCullFace(bool enabled);

    /**
     * @brief 设置剔除面
     * @param mode 剔除模式
     */
    void setCullFaceMode(GLenum mode);

    // 裁剪测试管理
    /**
     * @brief 设置裁剪测试
     * @param enabled 是否启用裁剪测试
     */
    void setScissorTest(bool enabled);

    /**
     * @brief 设置裁剪矩形
     * @param x 左下角X坐标
     * @param y 左下角Y坐标
     * @param width 宽度
     * @param height 高度
     */
    void setScissorRect(int x, int y, int width, int height);

    // 线框模式
    /**
     * @brief 设置线框模式
     * @param enabled 是否启用线框模式
     */
    void setWireframe(bool enabled);

    // 视口管理
    /**
     * @brief 设置视口
     * @param x 左下角X坐标
     * @param y 左下角Y坐标
     * @param width 宽度
     * @param height 高度
     */
    void setViewport(int x, int y, int width, int height);

    // 当前状态查询
    bool isBlendEnabled() const { return _blendEnabled; }
    bool isDepthTestEnabled() const { return _depthTestEnabled; }
    bool isStencilTestEnabled() const { return _stencilTestEnabled; }
    bool isCullFaceEnabled() const { return _cullFaceEnabled; }
    bool isScissorTestEnabled() const { return _scissorTestEnabled; }
    bool isWireframe() const { return _wireframeEnabled; }

    // 状态保存和恢复
    /**
     * @brief 保存当前所有状态
     */
    void pushState();

    /**
     * @brief 恢复之前保存的状态
     */
    void popState();

private:
    GLStateManager() = default;
    ~GLStateManager() = default;
    GLStateManager(const GLStateManager&) = delete;
    GLStateManager& operator=(const GLStateManager&) = delete;

private:
    // 混合状态
    bool _blendEnabled = false;
    GLenum _blendSrcFactor = GL_SRC_ALPHA;
    GLenum _blendDstFactor = GL_ONE_MINUS_SRC_ALPHA;
    GLenum _blendEquation = GL_FUNC_ADD;

    // 深度测试状态
    bool _depthTestEnabled = false;
    GLenum _depthFunc = GL_LESS;
    bool _depthWriteEnabled = true;

    // 模板测试状态
    bool _stencilTestEnabled = false;
    GLenum _stencilFunc = GL_ALWAYS;
    GLint _stencilRef = 0;
    GLuint _stencilMask = 0xFFFFFFFF;
    GLenum _stencilSFail = GL_KEEP;
    GLenum _stencilDPFail = GL_KEEP;
    GLenum _stencilDPPass = GL_KEEP;

    // 面剔除状态
    bool _cullFaceEnabled = false;
    GLenum _cullFaceMode = GL_BACK;

    // 裁剪测试状态
    bool _scissorTestEnabled = false;
    int _scissorX = 0;
    int _scissorY = 0;
    int _scissorWidth = 0;
    int _scissorHeight = 0;

    // 线框模式
    bool _wireframeEnabled = false;

    // 视口
    int _viewportX = 0;
    int _viewportY = 0;
    int _viewportWidth = 0;
    int _viewportHeight = 0;

    // 状态栈
    struct StateSnapshot
    {
        bool blendEnabled;
        GLenum blendSrcFactor;
        GLenum blendDstFactor;
        bool depthTestEnabled;
        bool scissorTestEnabled;
        int scissorX, scissorY, scissorWidth, scissorHeight;
        bool wireframeEnabled;
    };
    std::stack<StateSnapshot> _stateStack;
};

// 便捷宏定义
#define E2D_GL_STATE_MANAGER easy2d::GLStateManager::getInstance()

} // namespace easy2d
