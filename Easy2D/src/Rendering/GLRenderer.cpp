#define NOMINMAX
#include <easy2d/GLRenderer.h>
#include <easy2d/e2dbase.h>
#include <easy2d/e2dmanager.h>
#include <easy2d/e2dnode.h>
#include <easy2d/e2dtext.h>
#include <easy2d/e2dshape.h>
#include <easy2d/GLTextRenderer.h>

// WGL 扩展常量定义
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

// WGL 扩展函数指针类型
typedef HGLRC (WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int*);
typedef BOOL (WINAPI* PFNWGLSWAPINTERVALEXTPROC)(int);

namespace easy2d
{

GLRenderer& GLRenderer::getInstance()
{
    static GLRenderer instance;
    return instance;
}

bool GLRenderer::initialize(HWND hwnd, int width, int height)
{
    if (_initialized)
    {
        return true;
    }

    _hwnd = hwnd;
    _windowWidth = width;
    _windowHeight = height;

    // 初始化OpenGL上下文
    if (!initializeGLContext(hwnd))
    {
        E2D_ERROR("Failed to initialize OpenGL context");
        return false;
    }

    // 初始化GLAD
    if (!gladLoadGL())
    {
        E2D_ERROR("Failed to initialize GLAD");
        destroyGLContext();
        return false;
    }

    // 输出OpenGL信息
    E2D_LOG("OpenGL Version: %s", glGetString(GL_VERSION));
    E2D_LOG("OpenGL Vendor: %s", glGetString(GL_VENDOR));
    E2D_LOG("OpenGL Renderer: %s", glGetString(GL_RENDERER));
    E2D_LOG("GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // 初始化渲染目标
    E2D_GL_RENDER_TARGET.initialize(width, height);

    // 初始化着色器管理器
    if (!E2D_GL_SHADER_MANAGER.initialize())
    {
        E2D_ERROR("Failed to initialize shader manager");
        destroyGLContext();
        return false;
    }

    // 初始化文本渲染器
    if (!E2D_GL_TEXT_RENDERER.init())
    {
        E2D_ERROR("Failed to initialize text renderer");
        _shaderManager.shutdown();
        destroyGLContext();
        return false;
    }

    // 初始化默认资源
    if (!initializeResources())
    {
        E2D_ERROR("Failed to initialize resources");
        E2D_GL_TEXT_RENDERER.cleanup();
        _shaderManager.shutdown();
        destroyGLContext();
        return false;
    }

    // 设置默认投影矩阵（正交投影，原点在左上角）
    setProjection(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f);

    // 获取DPI缩放
    HDC hdc = GetDC(hwnd);
    _dpiScaleX = static_cast<float>(GetDeviceCaps(hdc, LOGPIXELSX));
    _dpiScaleY = static_cast<float>(GetDeviceCaps(hdc, LOGPIXELSY));
    ReleaseDC(hwnd, hdc);

    _initialized = true;
    E2D_LOG("GLRenderer initialized successfully");
    return true;
}

void GLRenderer::shutdown()
{
    if (!_initialized)
    {
        return;
    }

    // 销毁FPS文本
    if (_fpsText != nullptr)
    {
        delete _fpsText;
        _fpsText = nullptr;
    }

    // 销毁批次渲染器
    if (_batchRenderer != nullptr)
    {
        delete _batchRenderer;
        _batchRenderer = nullptr;
    }

    // 销毁默认纹理
    if (_defaultTexture != nullptr)
    {
        delete _defaultTexture;
        _defaultTexture = nullptr;
    }

    // 清空纹理缓存
    E2D_GL_TEXTURE_CACHE.clearCache();

    // 关闭文本渲染器
    E2D_GL_TEXT_RENDERER.cleanup();

    // 关闭着色器管理器
    E2D_GL_SHADER_MANAGER.shutdown();

    // 销毁OpenGL上下文
    destroyGLContext();

    _initialized = false;
    E2D_LOG("GLRenderer shutdown");
}

bool GLRenderer::initializeGLContext(HWND hwnd)
{
    // 获取设备上下文
    _hdc = GetDC(hwnd);
    if (!_hdc)
    {
        E2D_ERROR("Failed to get device context");
        return false;
    }

    // 设置像素格式描述符
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    // 选择像素格式
    int pixelFormat = ChoosePixelFormat(_hdc, &pfd);
    if (pixelFormat == 0)
    {
        E2D_ERROR("Failed to choose pixel format");
        ReleaseDC(hwnd, _hdc);
        _hdc = nullptr;
        return false;
    }

    // 设置像素格式
    if (!SetPixelFormat(_hdc, pixelFormat, &pfd))
    {
        E2D_ERROR("Failed to set pixel format");
        ReleaseDC(hwnd, _hdc);
        _hdc = nullptr;
        return false;
    }

    // 创建临时渲染上下文
    HGLRC tempContext = wglCreateContext(_hdc);
    if (!tempContext)
    {
        E2D_ERROR("Failed to create temporary OpenGL context");
        ReleaseDC(hwnd, _hdc);
        _hdc = nullptr;
        return false;
    }

    // 使临时上下文当前
    wglMakeCurrent(_hdc, tempContext);

    // 加载wgl扩展（需要临时上下文）
    // 注意：这里我们使用GLAD加载的OpenGL 3.3+，所以需要创建核心配置文件

    // 定义OpenGL 3.3核心配置文件属性
    int attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    // 创建现代OpenGL上下文（如果支持）
    HGLRC modernContext = nullptr;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    
    if (wglCreateContextAttribsARB)
    {
        modernContext = wglCreateContextAttribsARB(_hdc, 0, attribs);
    }

    // 删除临时上下文
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(tempContext);

    if (modernContext)
    {
        _hglrc = modernContext;
    }
    else
    {
        // 回退到传统上下文
        _hglrc = wglCreateContext(_hdc);
        if (!_hglrc)
        {
            E2D_ERROR("Failed to create OpenGL context");
            ReleaseDC(hwnd, _hdc);
            _hdc = nullptr;
            return false;
        }
    }

    // 使上下文当前
    if (!wglMakeCurrent(_hdc, _hglrc))
    {
        E2D_ERROR("Failed to make OpenGL context current");
        wglDeleteContext(_hglrc);
        _hglrc = nullptr;
        ReleaseDC(hwnd, _hdc);
        _hdc = nullptr;
        return false;
    }

    // 启用垂直同步
    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = 
        (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    if (wglSwapIntervalEXT)
    {
        wglSwapIntervalEXT(_vSyncEnabled ? 1 : 0);
    }

    return true;
}

void GLRenderer::destroyGLContext()
{
    if (_hglrc)
    {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(_hglrc);
        _hglrc = nullptr;
    }

    if (_hdc && _hwnd)
    {
        ReleaseDC(_hwnd, _hdc);
        _hdc = nullptr;
    }
}

bool GLRenderer::initializeResources()
{
    // 创建默认白色纹理（1x1像素）
    _defaultTexture = new GLTexture();
    unsigned char whitePixel[4] = { 255, 255, 255, 255 };
    if (!_defaultTexture->createFromRawData(1, 1, whitePixel, false))
    {
        E2D_ERROR("Failed to create default texture");
        return false;
    }

    // 创建批次渲染器
    _batchRenderer = new GLBatchRenderer();
    if (!_batchRenderer->initialize())
    {
        E2D_ERROR("Failed to initialize batch renderer");
        return false;
    }

    // 创建FPS显示文本
    _fpsText = new Text("FPS: 0");
    _fpsText->setAnchor(0, 0);
    _fpsText->setPos(0, 20);
    _fpsText->setScale(0.7f);

    return true;
}

void GLRenderer::resize(int width, int height)
{
    _windowWidth = width;
    _windowHeight = height;
    _renderTarget.resize(width, height);
    setProjection(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f);
}

void GLRenderer::beginFrame()
{
    // 绑定主渲染目标
    _renderTarget.bindMainTarget();

    // 清空屏幕
    clear(_clearColor);

    // 重置模型矩阵
    _modelMatrix.identity();
}

void GLRenderer::endFrame()
{
    // 渲染FPS（如果需要）
    if (_showFps)
    {
        renderFps();
    }

    // 交换缓冲区
    if (_hdc)
    {
        SwapBuffers(_hdc);
    }

    // 更新FPS统计
    _fpsFrameCount++;
}

void GLRenderer::clear(const Color& color)
{
    _renderTarget.clear(color.r, color.g, color.b, color.a);
}

void GLRenderer::setBackgroundColor(const Color& color)
{
    _backgroundColor = color;
    _clearColor = color;
    _renderTarget.setClearColor(color.r, color.g, color.b, color.a);
}

Color GLRenderer::getBackgroundColor() const
{
    return _backgroundColor;
}

void GLRenderer::setProjection(float left, float right, float bottom, float top)
{
    // 构建正交投影矩阵
    float tx = -(right + left) / (right - left);
    float ty = -(top + bottom) / (top - bottom);
    float tz = -1.0f;

    float ortho[16] = {
        2.0f / (right - left), 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        tx, ty, tz, 1.0f
    };

    memcpy(_projectionMatrix, ortho, sizeof(ortho));
}

void GLRenderer::setModelMatrix(const Matrix32& matrix)
{
    _modelMatrix = matrix;
}

void GLRenderer::setOpacity(float opacity)
{
    _opacity = opacity;
}

void GLRenderer::drawTexture(GLTexture* texture, const Rect& destRect, const Rect* srcRect, const Color& color)
{
    if (!texture || !texture->isValid())
    {
        return;
    }

    // 计算纹理坐标
    float u1 = 0.0f, v1 = 0.0f, u2 = 1.0f, v2 = 1.0f;
    if (srcRect)
    {
        float texWidth = static_cast<float>(texture->getWidth());
        float texHeight = static_cast<float>(texture->getHeight());
        u1 = srcRect->getLeft() / texWidth;
        v1 = srcRect->getTop() / texHeight;
        u2 = srcRect->getRight() / texWidth;
        v2 = srcRect->getBottom() / texHeight;
    }

    // 构建顶点
    Vertex vertices[4] = {
        Vertex(Point(destRect.getLeft(), destRect.getTop()), Point(u1, v1), color),
        Vertex(Point(destRect.getRight(), destRect.getTop()), Point(u2, v1), color),
        Vertex(Point(destRect.getRight(), destRect.getBottom()), Point(u2, v2), color),
        Vertex(Point(destRect.getLeft(), destRect.getBottom()), Point(u1, v2), color)
    };

    // 使用纹理着色器
    GLShader* shader = _shaderManager.getTextureShader();
    shader->use();
    shader->setMat4("uProjection", _projectionMatrix);
    shader->setMat4("uModel", _modelMatrix);
    shader->setFloat("uOpacity", _opacity);
    shader->setBool("uUseTexture", true);

    // 绑定纹理
    texture->bind(0);
    shader->setInt("uTexture", 0);

    // 创建临时缓冲区并绘制
    GLBuffer buffer;
    std::vector<Vertex> vertVec = { vertices[0], vertices[1], vertices[2], vertices[3] };
    std::vector<unsigned int> indices = { 0, 1, 2, 0, 2, 3 };
    buffer.create(vertVec, indices);
    buffer.draw();
}

void GLRenderer::drawFilledRect(const Rect& rect, const Color& color)
{
    // 构建顶点
    Vertex vertices[4] = {
        Vertex(Point(rect.getLeft(), rect.getTop()), Point(0, 0), color),
        Vertex(Point(rect.getRight(), rect.getTop()), Point(1, 0), color),
        Vertex(Point(rect.getRight(), rect.getBottom()), Point(1, 1), color),
        Vertex(Point(rect.getLeft(), rect.getBottom()), Point(0, 1), color)
    };

    // 使用颜色着色器
    GLShader* shader = _shaderManager.getColorShader();
    shader->use();
    shader->setMat4("uProjection", _projectionMatrix);
    shader->setMat4("uModel", _modelMatrix);
    shader->setFloat("uOpacity", _opacity);

    // 创建临时缓冲区并绘制
    GLBuffer buffer;
    std::vector<Vertex> vertVec = { vertices[0], vertices[1], vertices[2], vertices[3] };
    std::vector<unsigned int> indices = { 0, 1, 2, 0, 2, 3 };
    buffer.create(vertVec, indices);
    buffer.draw();
}

void GLRenderer::drawRect(const Rect& rect, const Color& color)
{
    drawFilledRect(rect, color);
}

void GLRenderer::drawRectOutline(const Rect& rect, const Color& color, float lineWidth)
{
    // 使用线条绘制矩形边框
    glLineWidth(lineWidth);
    
    Point points[5] = {
        Point(rect.getLeft(), rect.getTop()),
        Point(rect.getRight(), rect.getTop()),
        Point(rect.getRight(), rect.getBottom()),
        Point(rect.getLeft(), rect.getBottom()),
        Point(rect.getLeft(), rect.getTop())
    };

    // 使用颜色着色器
    GLShader* shader = _shaderManager.getColorShader();
    shader->use();
    shader->setMat4("uProjection", _projectionMatrix);
    shader->setMat4("uModel", _modelMatrix);
    shader->setFloat("uOpacity", _opacity);

    // 创建临时缓冲区并绘制
    std::vector<Vertex> vertices;
    for (int i = 0; i < 5; ++i)
    {
        vertices.emplace_back(points[i], Point(0, 0), color);
    }

    GLBuffer buffer;
    buffer.create(vertices, {}, true);
    buffer.bind();
    glDrawArrays(GL_LINE_STRIP, 0, 5);
    buffer.unbind();
}

void GLRenderer::drawFilledCircle(const Point& center, float radius, const Color& color)
{
    // 使用三角形扇绘制圆形
    const int segments = 32;
    std::vector<Vertex> vertices;
    vertices.reserve(segments + 2);

    // 中心点
    vertices.emplace_back(center, Point(0.5f, 0.5f), color);

    // 圆周上的点
    for (int i = 0; i <= segments; ++i)
    {
        float angle = 2.0f * 3.14159265359f * i / segments;
        float x = center.x + radius * cosf(angle);
        float y = center.y + radius * sinf(angle);
        vertices.emplace_back(Point(x, y), Point(0, 0), color);
    }

    // 使用颜色着色器
    GLShader* shader = _shaderManager.getColorShader();
    shader->use();
    shader->setMat4("uProjection", _projectionMatrix);
    shader->setMat4("uModel", _modelMatrix);
    shader->setFloat("uOpacity", _opacity);

    // 创建临时缓冲区并绘制
    GLBuffer buffer;
    buffer.create(vertices, {}, true);
    buffer.bind();
    glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<GLsizei>(vertices.size()));
    buffer.unbind();
}

void GLRenderer::drawCircleOutline(const Point& center, float radius, const Color& color, float lineWidth)
{
    glLineWidth(lineWidth);

    // 使用线段绘制圆形
    const int segments = 32;
    std::vector<Vertex> vertices;
    vertices.reserve(segments + 1);

    for (int i = 0; i <= segments; ++i)
    {
        float angle = 2.0f * 3.14159265359f * i / segments;
        float x = center.x + radius * cosf(angle);
        float y = center.y + radius * sinf(angle);
        vertices.emplace_back(Point(x, y), Point(0, 0), color);
    }

    // 使用颜色着色器
    GLShader* shader = _shaderManager.getColorShader();
    shader->use();
    shader->setMat4("uProjection", _projectionMatrix);
    shader->setMat4("uModel", _modelMatrix);
    shader->setFloat("uOpacity", _opacity);

    // 创建临时缓冲区并绘制
    GLBuffer buffer;
    buffer.create(vertices, {}, true);
    buffer.bind();
    glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(vertices.size()));
    buffer.unbind();
}

void GLRenderer::drawLine(const Point& start, const Point& end, const Color& color, float lineWidth)
{
    glLineWidth(lineWidth);

    Vertex vertices[2] = {
        Vertex(start, Point(0, 0), color),
        Vertex(end, Point(0, 0), color)
    };

    // 使用颜色着色器
    GLShader* shader = _shaderManager.getColorShader();
    shader->use();
    shader->setMat4("uProjection", _projectionMatrix);
    shader->setMat4("uModel", _modelMatrix);
    shader->setFloat("uOpacity", _opacity);

    // 创建临时缓冲区并绘制
    GLBuffer buffer;
    std::vector<Vertex> vertVec = { vertices[0], vertices[1] };
    buffer.create(vertVec, {}, true);
    buffer.bind();
    glDrawArrays(GL_LINES, 0, 2);
    buffer.unbind();
}

void GLRenderer::renderScene(bool showBodyShapes)
{
    // 调用场景管理器渲染
    SceneManager::__render(showBodyShapes);
}

void GLRenderer::renderFps()
{
    if (!_fpsText)
    {
        return;
    }

    // 更新FPS
    float currentTime = Time::getTotalTime();
    float deltaTime = currentTime - _fpsLastTime;

    if (deltaTime >= 0.5f)
    {
        _fpsCurrent = _fpsFrameCount / deltaTime;
        _fpsFrameCount = 0;
        _fpsLastTime = currentTime;

        // 更新FPS文本
        String fpsString = "FPS: " + std::to_string(static_cast<int>(_fpsCurrent));
        _fpsText->setText(fpsString);
    }

    // 渲染FPS文本
    _fpsText->_render();
}

void GLRenderer::showFps(bool show)
{
    _showFps = show;
}

void GLRenderer::showBodyShapes(bool show)
{
    _showBodyShapes = show;
}

void GLRenderer::setVSync(bool enabled)
{
    _vSyncEnabled = enabled;
    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = 
        (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    if (wglSwapIntervalEXT)
    {
        wglSwapIntervalEXT(enabled ? 1 : 0);
    }
}

void GLRenderer::drawTextLayout(TextLayout* layout, const Point& pos, const DrawingStyle& style)
{
    if (!layout)
    {
        return;
    }
    
    // 使用 GLTextRenderer 渲染文本
    // 从 TextLayout 获取实际的文本样式
    TextStyle textStyle = layout->getStyle();
    
    E2D_GL_TEXT_RENDERER.renderText(layout->getText(), pos, textStyle, style.fillColor);
}

} // namespace easy2d
