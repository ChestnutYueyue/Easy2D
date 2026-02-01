#pragma once
#include <easy2d/e2dmacros.h>
#include <glad/glad.h>
#include <string>

namespace easy2d
{

/**
 * @brief OpenGL调试工具类
 * 
 * 提供OpenGL错误检查、调试信息输出等功能
 */
class GLDebug
{
public:
    /**
     * @brief 检查OpenGL错误
     * @param file 源文件名
     * @param line 行号
     * @param function 函数名
     * @return 是否有错误
     */
    static bool checkError(const char* file, int line, const char* function);

    /**
     * @brief 获取OpenGL错误字符串
     * @param error 错误代码
     * @return 错误描述字符串
     */
    static const char* getErrorString(GLenum error);

    /**
     * @brief 启用OpenGL调试输出（需要OpenGL 4.3+）
     */
    static void enableDebugOutput();

    /**
     * @brief 禁用OpenGL调试输出
     */
    static void disableDebugOutput();

    /**
     * @brief 检查帧缓冲完整性
     * @param target 帧缓冲目标
     * @return 是否完整
     */
    static bool checkFramebufferStatus(GLenum target);

    /**
     * @brief 获取OpenGL版本信息
     * @return 版本信息字符串
     */
    static std::string getVersionInfo();

    /**
     * @brief 获取GPU信息
     * @return GPU信息字符串
     */
    static std::string getGPUInfo();

    /**
     * @brief 检查是否支持特定扩展
     * @param extension 扩展名
     * @return 是否支持
     */
    static bool isExtensionSupported(const char* extension);

    /**
     * @brief 获取最大纹理尺寸
     * @return 最大纹理尺寸
     */
    static GLint getMaxTextureSize();

    /**
     * @brief 获取最大渲染缓冲尺寸
     * @return 最大渲染缓冲尺寸
     */
    static GLint getMaxRenderbufferSize();

    /**
     * @brief 获取最大视口尺寸
     * @return 最大视口尺寸
     */
    static GLint getMaxViewportDims();

    /**
     * @brief 获取最大顶点属性数量
     * @return 最大顶点属性数量
     */
    static GLint getMaxVertexAttribs();

    /**
     * @brief 获取最大纹理单元数量
     * @return 最大纹理单元数量
     */
    static GLint getMaxTextureImageUnits();

    /**
     * @brief 打印所有OpenGL限制信息
     */
    static void printGLInfo();
};

// 调试宏定义
#ifdef E2D_DEBUG
    #define GL_CHECK() easy2d::GLDebug::checkError(__FILE__, __LINE__, __FUNCTION__)
    #define GL_CALL(func) do { func; GL_CHECK(); } while(0)
#else
    #define GL_CHECK() ((void)0)
    #define GL_CALL(func) func
#endif

} // namespace easy2d
