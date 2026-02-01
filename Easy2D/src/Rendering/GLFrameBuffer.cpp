#include <easy2d/GLFrameBuffer.h>
#include <easy2d/e2dbase.h>

namespace easy2d
{

// GLFrameBuffer 实现

GLFrameBuffer::GLFrameBuffer()
    : _frameBufferID(0)
    , _depthStencilID(0)
    , _colorTexture(nullptr)
    , _width(0)
    , _height(0)
    , _hasDepthStencil(false)
{
}

GLFrameBuffer::~GLFrameBuffer()
{
    release();
}

void GLFrameBuffer::release()
{
    if (_frameBufferID != 0)
    {
        glDeleteFramebuffers(1, &_frameBufferID);
        _frameBufferID = 0;
    }
    if (_depthStencilID != 0)
    {
        glDeleteRenderbuffers(1, &_depthStencilID);
        _depthStencilID = 0;
    }
    if (_colorTexture != nullptr)
    {
        delete _colorTexture;
        _colorTexture = nullptr;
    }
    _width = 0;
    _height = 0;
}

bool GLFrameBuffer::create(int width, int height, bool hasDepthStencil)
{
    release();

    _width = width;
    _height = height;
    _hasDepthStencil = hasDepthStencil;

    // 创建帧缓冲
    glGenFramebuffers(1, &_frameBufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferID);

    // 创建颜色纹理
    _colorTexture = new GLTexture();
    if (!_colorTexture->createEmpty(width, height, GL_RGBA))
    {
        E2D_ERROR("Failed to create color texture for framebuffer");
        release();
        return false;
    }

    // 将颜色纹理附加到帧缓冲
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorTexture->getID(), 0);

    // 创建深度/模板缓冲（如果需要）
    if (hasDepthStencil)
    {
        glGenRenderbuffers(1, &_depthStencilID);
        glBindRenderbuffer(GL_RENDERBUFFER, _depthStencilID);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        // 将深度/模板缓冲附加到帧缓冲
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthStencilID);
    }

    // 检查帧缓冲是否完整
    if (!checkStatus())
    {
        E2D_ERROR("Framebuffer is not complete");
        release();
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

bool GLFrameBuffer::resize(int width, int height)
{
    if (_frameBufferID == 0)
    {
        return create(width, height, _hasDepthStencil);
    }

    // 重新创建纹理
    if (_colorTexture != nullptr)
    {
        _colorTexture->createEmpty(width, height, GL_RGBA);
    }

    // 重新创建深度/模板缓冲
    if (_hasDepthStencil && _depthStencilID != 0)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, _depthStencilID);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    _width = width;
    _height = height;

    // 检查帧缓冲状态
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferID);
    bool complete = checkStatus();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return complete;
}

void GLFrameBuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferID);
    glViewport(0, 0, _width, _height);
}

void GLFrameBuffer::unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLFrameBuffer::clear(float r, float g, float b, float a) const
{
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferID);
    glClearColor(r, g, b, a);
    if (_hasDepthStencil)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    else
    {
        glClear(GL_COLOR_BUFFER_BIT);
    }
}

bool GLFrameBuffer::isComplete() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferID);
    bool complete = checkStatus();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return complete;
}

bool GLFrameBuffer::checkStatus() const
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        const char* errorMsg = "Unknown framebuffer error";
        switch (status)
        {
        case GL_FRAMEBUFFER_UNDEFINED:
            errorMsg = "Framebuffer undefined";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            errorMsg = "Framebuffer incomplete attachment";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            errorMsg = "Framebuffer missing attachment";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            errorMsg = "Framebuffer incomplete draw buffer";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            errorMsg = "Framebuffer incomplete read buffer";
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            errorMsg = "Framebuffer unsupported";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            errorMsg = "Framebuffer incomplete multisample";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            errorMsg = "Framebuffer incomplete layer targets";
            break;
        }
        E2D_ERROR("Framebuffer status check failed: %s", errorMsg);
        return false;
    }
    return true;
}

// GLRenderTarget 实现

GLRenderTarget& GLRenderTarget::getInstance()
{
    static GLRenderTarget instance;
    return instance;
}

void GLRenderTarget::initialize(int windowWidth, int windowHeight)
{
    _windowWidth = windowWidth;
    _windowHeight = windowHeight;

    // 设置默认视口
    setDefaultViewport();

    // 启用混合
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    E2D_LOG("GLRenderTarget initialized: %dx%d", windowWidth, windowHeight);
}

void GLRenderTarget::resize(int width, int height)
{
    _windowWidth = width;
    _windowHeight = height;
    setDefaultViewport();
}

void GLRenderTarget::bindMainTarget() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    setDefaultViewport();
}

void GLRenderTarget::setViewport(int x, int y, int width, int height) const
{
    glViewport(x, y, width, height);
}

void GLRenderTarget::setDefaultViewport() const
{
    glViewport(0, 0, _windowWidth, _windowHeight);
}

void GLRenderTarget::clear(float r, float g, float b, float a) const
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void GLRenderTarget::setClearColor(float r, float g, float b, float a)
{
    _clearColor[0] = r;
    _clearColor[1] = g;
    _clearColor[2] = b;
    _clearColor[3] = a;
}

} // namespace easy2d
