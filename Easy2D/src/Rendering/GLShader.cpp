#include <easy2d/GLShader.h>
#include <easy2d/e2dbase.h>
#include <fstream>
#include <sstream>
#include <iostream>

namespace easy2d
{

// 内置着色器源码

// 基础纹理着色器
static const char* TEXTURE_VERTEX_SHADER = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec4 aColor;

out vec2 TexCoord;
out vec4 VertexColor;

uniform mat4 uProjection;
uniform mat4 uModel;

void main()
{
    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
    VertexColor = aColor;
}
)";

static const char* TEXTURE_FRAGMENT_SHADER = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec4 VertexColor;

uniform sampler2D uTexture;
uniform float uOpacity;
uniform bool uUseTexture;
uniform vec4 uColor;

void main()
{
    vec4 color;
    if (uUseTexture)
    {
        color = texture(uTexture, TexCoord) * VertexColor;
    }
    else
    {
        color = uColor * VertexColor;
    }
    color.a *= uOpacity;
    FragColor = color;
}
)";

// 基础颜色着色器
static const char* COLOR_VERTEX_SHADER = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;

out vec4 VertexColor;

uniform mat4 uProjection;
uniform mat4 uModel;

void main()
{
    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
    VertexColor = aColor;
}
)";

static const char* COLOR_FRAGMENT_SHADER = R"(
#version 330 core
out vec4 FragColor;

in vec4 VertexColor;

uniform float uOpacity;

void main()
{
    FragColor = vec4(VertexColor.rgb, VertexColor.a * uOpacity);
}
)";

// 文本渲染着色器（使用锐利的alpha阈值）
static const char* TEXT_VERTEX_SHADER = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec4 aColor;

out vec2 TexCoord;
out vec4 TextColor;

uniform mat4 uProjection;
uniform mat4 uModel;

void main()
{
    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
    TextColor = aColor;
}
)";

static const char* TEXT_FRAGMENT_SHADER = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec4 TextColor;

uniform sampler2D uTexture;
uniform float uOpacity;
uniform vec4 uOutlineColor;
uniform float uOutlineWidth;
uniform bool uHasOutline;

void main()
{
    // 读取alpha通道
    float alpha = texture(uTexture, TexCoord).a;
    
    // 使用更锐利的阈值处理，避免边缘模糊
    // 对于NEAREST过滤的纹理，使用简单的阈值即可
    float sharpAlpha = alpha > 0.5 ? 1.0 : 0.0;
    
    vec4 color = TextColor;
    color.a *= sharpAlpha * uOpacity;
    
    if (uHasOutline && uOutlineWidth > 0.0)
    {
        // 轮廓使用稍宽的阈值范围
        float outlineAlpha = alpha > 0.3 ? 1.0 : 0.0;
        vec4 outline = uOutlineColor;
        outline.a *= outlineAlpha * uOpacity;
        color = mix(outline, color, sharpAlpha);
    }
    
    FragColor = color;
}
)";

// 形状渲染着色器
static const char* SHAPE_VERTEX_SHADER = R"(
#version 330 core
layout (location = 0) in vec2 aPos;

uniform mat4 uProjection;
uniform mat4 uModel;

void main()
{
    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
}
)";

static const char* SHAPE_FRAGMENT_SHADER = R"(
#version 330 core
out vec4 FragColor;

uniform vec4 uFillColor;
uniform vec4 uStrokeColor;
uniform float uOpacity;
uniform bool uIsStroke;

void main()
{
    vec4 color = uIsStroke ? uStrokeColor : uFillColor;
    color.a *= uOpacity;
    FragColor = color;
}
)";

// GLShader 实现

GLShader::GLShader()
    : _programID(0)
{
}

GLShader::~GLShader()
{
    if (_programID != 0)
    {
        glDeleteProgram(_programID);
        _programID = 0;
    }
}

bool GLShader::loadFromSource(const std::string& vertexSource, const std::string& fragmentSource)
{
    // 编译顶点着色器
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    if (vertexShader == 0)
    {
        E2D_ERROR("Failed to compile vertex shader");
        return false;
    }

    // 编译片段着色器
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (fragmentShader == 0)
    {
        E2D_ERROR("Failed to compile fragment shader");
        glDeleteShader(vertexShader);
        return false;
    }

    // 链接着色器程序
    bool success = linkProgram(vertexShader, fragmentShader);

    // 删除着色器对象（已链接到程序中）
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return success;
}

bool GLShader::loadFromFile(const std::string& vertexPath, const std::string& fragmentPath)
{
    // 读取顶点着色器文件
    std::ifstream vertexFile(vertexPath);
    if (!vertexFile.is_open())
    {
        E2D_ERROR("Failed to open vertex shader file: %s", vertexPath.c_str());
        return false;
    }
    std::stringstream vertexStream;
    vertexStream << vertexFile.rdbuf();
    vertexFile.close();

    // 读取片段着色器文件
    std::ifstream fragmentFile(fragmentPath);
    if (!fragmentFile.is_open())
    {
        E2D_ERROR("Failed to open fragment shader file: %s", fragmentPath.c_str());
        return false;
    }
    std::stringstream fragmentStream;
    fragmentStream << fragmentFile.rdbuf();
    fragmentFile.close();

    return loadFromSource(vertexStream.str(), fragmentStream.str());
}

void GLShader::use() const
{
    glUseProgram(_programID);
}

GLuint GLShader::compileShader(GLenum type, const std::string& source)
{
    GLuint shader = glCreateShader(type);
    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);

    // 检查编译结果
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        E2D_ERROR("Shader compilation failed: %s", infoLog);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool GLShader::linkProgram(GLuint vertexShader, GLuint fragmentShader)
{
    _programID = glCreateProgram();
    glAttachShader(_programID, vertexShader);
    glAttachShader(_programID, fragmentShader);
    glLinkProgram(_programID);

    // 检查链接结果
    GLint success;
    glGetProgramiv(_programID, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(_programID, 512, nullptr, infoLog);
        E2D_ERROR("Shader program linking failed: %s", infoLog);
        glDeleteProgram(_programID);
        _programID = 0;
        return false;
    }

    return true;
}

GLint GLShader::getUniformLocation(const std::string& name) const
{
    // 查找缓存
    auto it = _uniformCache.find(name);
    if (it != _uniformCache.end())
    {
        return it->second;
    }

    // 获取uniform位置
    GLint location = glGetUniformLocation(_programID, name.c_str());
    _uniformCache[name] = location;
    return location;
}

void GLShader::setBool(const std::string& name, bool value) const
{
    glUniform1i(getUniformLocation(name), static_cast<int>(value));
}

void GLShader::setInt(const std::string& name, int value) const
{
    glUniform1i(getUniformLocation(name), value);
}

void GLShader::setFloat(const std::string& name, float value) const
{
    glUniform1f(getUniformLocation(name), value);
}

void GLShader::setVec2(const std::string& name, float x, float y) const
{
    glUniform2f(getUniformLocation(name), x, y);
}

void GLShader::setVec3(const std::string& name, float x, float y, float z) const
{
    glUniform3f(getUniformLocation(name), x, y, z);
}

void GLShader::setVec4(const std::string& name, float x, float y, float z, float w) const
{
    glUniform4f(getUniformLocation(name), x, y, z, w);
}

void GLShader::setMat3(const std::string& name, const float* matrix) const
{
    glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, matrix);
}

void GLShader::setMat4(const std::string& name, const float* matrix) const
{
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, matrix);
}

void GLShader::setMat4(const std::string& name, const Matrix32& matrix) const
{
    // 将2D矩阵转换为4x4矩阵
    float mat4[16] = {
        matrix._11, matrix._12, 0.0f, 0.0f,
        matrix._21, matrix._22, 0.0f, 0.0f,
        0.0f,       0.0f,       1.0f, 0.0f,
        matrix._31, matrix._32, 0.0f, 1.0f
    };
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, mat4);
}

// GLShaderManager 实现

GLShaderManager& GLShaderManager::getInstance()
{
    static GLShaderManager instance;
    return instance;
}

bool GLShaderManager::initialize()
{
    // 创建纹理着色器
    _textureShader = new GLShader();
    if (!_textureShader->loadFromSource(TEXTURE_VERTEX_SHADER, TEXTURE_FRAGMENT_SHADER))
    {
        E2D_ERROR("Failed to initialize texture shader");
        return false;
    }

    // 创建颜色着色器
    _colorShader = new GLShader();
    if (!_colorShader->loadFromSource(COLOR_VERTEX_SHADER, COLOR_FRAGMENT_SHADER))
    {
        E2D_ERROR("Failed to initialize color shader");
        return false;
    }

    // 创建文本着色器
    _textShader = new GLShader();
    if (!_textShader->loadFromSource(TEXT_VERTEX_SHADER, TEXT_FRAGMENT_SHADER))
    {
        E2D_ERROR("Failed to initialize text shader");
        return false;
    }

    // 创建形状着色器
    _shapeShader = new GLShader();
    if (!_shapeShader->loadFromSource(SHAPE_VERTEX_SHADER, SHAPE_FRAGMENT_SHADER))
    {
        E2D_ERROR("Failed to initialize shape shader");
        return false;
    }

    E2D_LOG("GLShaderManager initialized successfully");
    return true;
}

void GLShaderManager::shutdown()
{
    delete _textureShader;
    _textureShader = nullptr;

    delete _colorShader;
    _colorShader = nullptr;

    delete _textShader;
    _textShader = nullptr;

    delete _shapeShader;
    _shapeShader = nullptr;

    E2D_LOG("GLShaderManager shutdown");
}

} // namespace easy2d
