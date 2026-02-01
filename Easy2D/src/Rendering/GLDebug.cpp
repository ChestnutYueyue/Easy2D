#include <easy2d/GLDebug.h>
#include <easy2d/e2dbase.h>
#include <sstream>

namespace easy2d
{

// OpenGL调试回调函数（需要OpenGL 4.3+）
static void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity,
                                    GLsizei length, const GLchar* message, const void* userParam)
{
    // 忽略某些不重要的消息
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    const char* sourceStr = "Unknown";
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             sourceStr = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sourceStr = "Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     sourceStr = "Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     sourceStr = "Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           sourceStr = "Other"; break;
    }

    const char* typeStr = "Unknown";
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               typeStr = "Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behavior"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeStr = "Undefined Behavior"; break;
        case GL_DEBUG_TYPE_PORTABILITY:         typeStr = "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         typeStr = "Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              typeStr = "Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          typeStr = "Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           typeStr = "Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               typeStr = "Other"; break;
    }

    const char* severityStr = "Unknown";
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         severityStr = "High"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       severityStr = "Medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          severityStr = "Low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr = "Notification"; break;
    }

    E2D_LOG("[OpenGL Debug] Source: %s, Type: %s, Severity: %s, ID: %u\n  Message: %s",
            sourceStr, typeStr, severityStr, id, message);
}

bool GLDebug::checkError(const char* file, int line, const char* function)
{
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        E2D_ERROR("[OpenGL Error] (%s) at %s:%d in %s", 
                  getErrorString(error), file, line, function);
        return false;
    }
    return true;
}

const char* GLDebug::getErrorString(GLenum error)
{
    switch (error)
    {
        case GL_NO_ERROR:                      return "NO_ERROR";
        case GL_INVALID_ENUM:                  return "INVALID_ENUM";
        case GL_INVALID_VALUE:                 return "INVALID_VALUE";
        case GL_INVALID_OPERATION:             return "INVALID_OPERATION";
        case GL_STACK_OVERFLOW:                return "STACK_OVERFLOW";
        case GL_STACK_UNDERFLOW:               return "STACK_UNDERFLOW";
        case GL_OUT_OF_MEMORY:                 return "OUT_OF_MEMORY";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "INVALID_FRAMEBUFFER_OPERATION";
        default:                               return "UNKNOWN_ERROR";
    }
}

void GLDebug::enableDebugOutput()
{
    // 检查OpenGL版本是否支持调试输出（4.3+）
    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    if (major > 4 || (major == 4 && minor >= 3))
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        E2D_LOG("OpenGL debug output enabled");
    }
    else
    {
        E2D_LOG("OpenGL debug output not supported (requires 4.3+)");
    }
}

void GLDebug::disableDebugOutput()
{
    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    if (major > 4 || (major == 4 && minor >= 3))
    {
        glDisable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(nullptr, nullptr);
    }
}

bool GLDebug::checkFramebufferStatus(GLenum target)
{
    GLenum status = glCheckFramebufferStatus(target);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        const char* errorMsg = "Unknown framebuffer error";
        switch (status)
        {
            case GL_FRAMEBUFFER_UNDEFINED:                     errorMsg = "UNDEFINED"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:         errorMsg = "INCOMPLETE_ATTACHMENT"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: errorMsg = "MISSING_ATTACHMENT"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:        errorMsg = "INCOMPLETE_DRAW_BUFFER"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:        errorMsg = "INCOMPLETE_READ_BUFFER"; break;
            case GL_FRAMEBUFFER_UNSUPPORTED:                   errorMsg = "UNSUPPORTED"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:        errorMsg = "INCOMPLETE_MULTISAMPLE"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:      errorMsg = "INCOMPLETE_LAYER_TARGETS"; break;
        }
        E2D_ERROR("Framebuffer incomplete: %s", errorMsg);
        return false;
    }
    return true;
}

std::string GLDebug::getVersionInfo()
{
    std::ostringstream oss;
    oss << "OpenGL Version: " << reinterpret_cast<const char*>(glGetString(GL_VERSION)) << "\n";
    oss << "GLSL Version: " << reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)) << "\n";
    oss << "OpenGL Vendor: " << reinterpret_cast<const char*>(glGetString(GL_VENDOR)) << "\n";
    oss << "OpenGL Renderer: " << reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    return oss.str();
}

std::string GLDebug::getGPUInfo()
{
    std::ostringstream oss;
    oss << "GPU: " << reinterpret_cast<const char*>(glGetString(GL_RENDERER)) << "\n";
    oss << "Vendor: " << reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    return oss.str();
}

bool GLDebug::isExtensionSupported(const char* extension)
{
    GLint numExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    
    for (GLint i = 0; i < numExtensions; ++i)
    {
        const char* ext = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
        if (strcmp(ext, extension) == 0)
        {
            return true;
        }
    }
    return false;
}

GLint GLDebug::getMaxTextureSize()
{
    GLint value;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
    return value;
}

GLint GLDebug::getMaxRenderbufferSize()
{
    GLint value;
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &value);
    return value;
}

GLint GLDebug::getMaxViewportDims()
{
    GLint value;
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, &value);
    return value;
}

GLint GLDebug::getMaxVertexAttribs()
{
    GLint value;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &value);
    return value;
}

GLint GLDebug::getMaxTextureImageUnits()
{
    GLint value;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &value);
    return value;
}

void GLDebug::printGLInfo()
{
    E2D_LOG("=== OpenGL Information ===");
    E2D_LOG("Version: %s", glGetString(GL_VERSION));
    E2D_LOG("GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    E2D_LOG("Vendor: %s", glGetString(GL_VENDOR));
    E2D_LOG("Renderer: %s", glGetString(GL_RENDERER));
    E2D_LOG("Max Texture Size: %d", getMaxTextureSize());
    E2D_LOG("Max Renderbuffer Size: %d", getMaxRenderbufferSize());
    E2D_LOG("Max Viewport Dims: %d", getMaxViewportDims());
    E2D_LOG("Max Vertex Attribs: %d", getMaxVertexAttribs());
    E2D_LOG("Max Texture Image Units: %d", getMaxTextureImageUnits());
    E2D_LOG("==========================");
}

} // namespace easy2d
