#pragma once
#include <easy2d/e2dmacros.h>
#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include <easy2d/e2dmath.h>

namespace easy2d
{

/**
 * @brief OpenGL着色器程序类
 * 
 * 封装了OpenGL着色器的编译、链接和uniform设置功能
 * 替代D2D1中内置的渲染管线
 */
class GLShader
{
public:
    /**
     * @brief 构造函数
     */
    GLShader();

    /**
     * @brief 析构函数，自动释放着色器资源
     */
    ~GLShader();

    /**
     * @brief 从字符串加载顶点和片段着色器
     * @param vertexSource 顶点着色器源码
     * @param fragmentSource 片段着色器源码
     * @return 是否加载成功
     */
    bool loadFromSource(const std::string& vertexSource, const std::string& fragmentSource);

    /**
     * @brief 从文件加载顶点和片段着色器
     * @param vertexPath 顶点着色器文件路径
     * @param fragmentPath 片段着色器文件路径
     * @return 是否加载成功
     */
    bool loadFromFile(const std::string& vertexPath, const std::string& fragmentPath);

    /**
     * @brief 激活此着色器程序
     */
    void use() const;

    /**
     * @brief 获取着色器程序ID
     * @return OpenGL程序ID
     */
    GLuint getProgramID() const { return _programID; }

    /**
     * @brief 设置uniform变量（bool）
     */
    void setBool(const std::string& name, bool value) const;

    /**
     * @brief 设置uniform变量（int）
     */
    void setInt(const std::string& name, int value) const;

    /**
     * @brief 设置uniform变量（float）
     */
    void setFloat(const std::string& name, float value) const;

    /**
     * @brief 设置uniform变量（vec2）
     */
    void setVec2(const std::string& name, float x, float y) const;

    /**
     * @brief 设置uniform变量（vec3）
     */
    void setVec3(const std::string& name, float x, float y, float z) const;

    /**
     * @brief 设置uniform变量（vec4）
     */
    void setVec4(const std::string& name, float x, float y, float z, float w) const;

    /**
     * @brief 设置uniform变量（mat3）
     */
    void setMat3(const std::string& name, const float* matrix) const;

    /**
     * @brief 设置uniform变量（mat4）
     */
    void setMat4(const std::string& name, const float* matrix) const;

    /**
     * @brief 设置uniform变量（mat4，使用Matrix32）
     */
    void setMat4(const std::string& name, const Matrix32& matrix) const;

private:
    /**
     * @brief 编译单个着色器
     * @param type 着色器类型（GL_VERTEX_SHADER/GL_FRAGMENT_SHADER）
     * @param source 着色器源码
     * @return 着色器ID，失败返回0
     */
    GLuint compileShader(GLenum type, const std::string& source);

    /**
     * @brief 链接着色器程序
     * @param vertexShader 顶点着色器ID
     * @param fragmentShader 片段着色器ID
     * @return 是否链接成功
     */
    bool linkProgram(GLuint vertexShader, GLuint fragmentShader);

    /**
     * @brief 获取uniform位置（带缓存）
     * @param name uniform变量名
     * @return uniform位置
     */
    GLint getUniformLocation(const std::string& name) const;

private:
    GLuint _programID;                                          // 着色器程序ID
    mutable std::unordered_map<std::string, GLint> _uniformCache; // uniform位置缓存
};

/**
 * @brief 着色器管理器（单例）
 * 
 * 管理所有预设着色器的生命周期
 */
class GLShaderManager
{
public:
    /**
     * @brief 获取单例实例
     */
    static GLShaderManager& getInstance();

    /**
     * @brief 初始化所有预设着色器
     * @return 是否初始化成功
     */
    bool initialize();

    /**
     * @brief 销毁所有着色器资源
     */
    void shutdown();

    /**
     * @brief 获取基础纹理着色器
     */
    GLShader* getTextureShader() const { return _textureShader; }

    /**
     * @brief 获取基础颜色着色器
     */
    GLShader* getColorShader() const { return _colorShader; }

    /**
     * @brief 获取文本渲染着色器
     */
    GLShader* getTextShader() const { return _textShader; }

    /**
     * @brief 获取形状渲染着色器
     */
    GLShader* getShapeShader() const { return _shapeShader; }

private:
    GLShaderManager() = default;
    ~GLShaderManager() = default;
    GLShaderManager(const GLShaderManager&) = delete;
    GLShaderManager& operator=(const GLShaderManager&) = delete;

private:
    GLShader* _textureShader = nullptr;  // 纹理着色器
    GLShader* _colorShader = nullptr;    // 颜色着色器
    GLShader* _textShader = nullptr;     // 文本着色器
    GLShader* _shapeShader = nullptr;    // 形状着色器
};

// 便捷宏定义
#define E2D_GL_SHADER_MANAGER easy2d::GLShaderManager::getInstance()

} // namespace easy2d
