#define NOMINMAX
#include <easy2d/GLRenderer.h>
#include <easy2d/GLTextureAtlas.h>
#include <easy2d/e2dbase.h>
#include <easy2d/e2dmanager.h>
#include <easy2d/e2dnode.h>
#include <easy2d/e2dtext.h>
#include <easy2d/e2dshape.h>
#include <easy2d/GLTextRenderer.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

namespace easy2d
{

GLRenderer& GLRenderer::getInstance()
{
    static GLRenderer instance;
    return instance;
}

bool GLRenderer::initialize(SDL_Window* window, int width, int height)
{
    if (_initialized)
    {
        return true;
    }

    _window = window;
    _windowWidth = width;
    _windowHeight = height;

    // 初始化OpenGL上下文
    if (!initializeGLContext(window))
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

    // 初始化纹理图集系统
    E2D_GL_TEXTURE_ATLAS.initialize(2048, 2048, 4);

    // 设置默认投影矩阵（正交投影，原点在左上角）
    setProjection(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f);

    // 获取DPI缩放 (SDL3使用SDL_GetWindowDisplayScale)
    float scale = SDL_GetWindowDisplayScale(window);
    if (scale > 0.0f)
    {
        _dpiScaleX = 96.0f * scale;
        _dpiScaleY = 96.0f * scale;
    }
    else
    {
        _dpiScaleX = 96.0f;
        _dpiScaleY = 96.0f;
    }

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

    // 关闭纹理图集系统
    E2D_GL_TEXTURE_ATLAS.shutdown();

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

bool GLRenderer::initializeGLContext(SDL_Window* window)
{
    // 设置OpenGL属性
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // 创建OpenGL上下文
    _glContext = SDL_GL_CreateContext(window);
    if (!_glContext)
    {
        E2D_ERROR("Failed to create OpenGL context: %s", SDL_GetError());
        return false;
    }

    // 使上下文当前
    if (!SDL_GL_MakeCurrent(window, _glContext))
    {
        E2D_ERROR("Failed to make OpenGL context current: %s", SDL_GetError());
        SDL_GL_DestroyContext(_glContext);
        _glContext = nullptr;
        return false;
    }

    // 启用垂直同步
    SDL_GL_SetSwapInterval(_vSyncEnabled ? 1 : 0);

    return true;
}

void GLRenderer::destroyGLContext()
{
    if (_glContext)
    {
        SDL_GL_DestroyContext(_glContext);
        _glContext = nullptr;
    }
    _window = nullptr;
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

    // 开始批次渲染
    if (_batchRenderer)
    {
        _batchRenderer->begin();
    }
}

void GLRenderer::endFrame()
{
    // 结束批次渲染，提交所有待渲染的精灵
    if (_batchRenderer)
    {
        _batchRenderer->end();
    }

    // 交换缓冲区
    if (_window)
    {
        SDL_GL_SwapWindow(_window);
    }

    // 更新FPS统计（使用SDL2高精度计时器）
    if (_showFps)
    {
        _fpsFrameCount++;
        Uint64 currentTime = SDL_GetPerformanceCounter();
        Uint64 frequency = SDL_GetPerformanceFrequency();
        double deltaTime = static_cast<double>(currentTime - _fpsLastTime) / frequency;

        if (deltaTime >= 0.5) // 每0.5秒更新一次FPS显示
        {
            double fps = _fpsFrameCount / deltaTime;
            _fpsFrameCount = 0;
            _fpsLastTime = currentTime;

            // 更新窗口标题显示FPS
            std::string titleWithFps = _windowTitle + " - FPS: " + std::to_string(static_cast<int>(fps));
            SDL_SetWindowTitle(_window, titleWithFps.c_str());
        }
    }
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
    // 使用 GLM 构建正交投影矩阵
    glm::mat4 ortho = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);

    memcpy(_projectionMatrix, glm::value_ptr(ortho), sizeof(_projectionMatrix));

    // 同时更新批次渲染器的投影矩阵
    if (_batchRenderer)
    {
        _batchRenderer->setProjectionMatrix(_projectionMatrix);
    }
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
    
    // 尝试使用纹理图集
    const AtlasRegion* atlasRegion = E2D_GL_TEXTURE_ATLAS.getRegion(texture);
    if (atlasRegion)
    {
        // 使用图集中的UV坐标
        if (srcRect)
        {
            // 计算在图集中的相对UV坐标
            float texWidth = static_cast<float>(texture->getWidth());
            float texHeight = static_cast<float>(texture->getHeight());
            float srcU1 = srcRect->getLeft() / texWidth;
            float srcV1 = srcRect->getTop() / texHeight;
            float srcU2 = srcRect->getRight() / texWidth;
            float srcV2 = srcRect->getBottom() / texHeight;
            
            // 映射到图集UV空间
            float regionWidth = atlasRegion->uv1.x - atlasRegion->uv0.x;
            float regionHeight = atlasRegion->uv1.y - atlasRegion->uv0.y;
            
            u1 = atlasRegion->uv0.x + srcU1 * regionWidth;
            v1 = atlasRegion->uv0.y + srcV1 * regionHeight;
            u2 = atlasRegion->uv0.x + srcU2 * regionWidth;
            v2 = atlasRegion->uv0.y + srcV2 * regionHeight;
        }
        else
        {
            u1 = atlasRegion->uv0.x;
            v1 = atlasRegion->uv0.y;
            u2 = atlasRegion->uv1.x;
            v2 = atlasRegion->uv1.y;
        }
        
        // 获取图集页面纹理
        GLTextureAtlasPage* page = E2D_GL_TEXTURE_ATLAS.getPage(atlasRegion->pageIndex);
        if (page)
        {
            texture = page->getTexture();
        }
    }
    else if (srcRect)
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

    // 应用模型矩阵变换到顶点位置
    for (int i = 0; i < 4; ++i)
    {
        vertices[i].position = _modelMatrix.transformPoint(vertices[i].position);
    }

    // 应用透明度
    Color finalColor = color;
    finalColor.a *= _opacity;
    for (int i = 0; i < 4; ++i)
    {
        vertices[i].color = finalColor;
    }

    // 使用批次渲染器添加四边形
    if (_batchRenderer)
    {
        _batchRenderer->addQuad(vertices, texture->getID());
    }
    else
    {
        // 回退到直接渲染（不应该发生）
        GLShader* shader = _shaderManager.getTextureShader();
        shader->use();
        shader->setMat4("uProjection", _projectionMatrix);
        shader->setMat4("uModel", _modelMatrix);
        shader->setFloat("uOpacity", _opacity);
        shader->setBool("uUseTexture", true);

        texture->bind(0);
        shader->setInt("uTexture", 0);

        GLBuffer buffer;
        std::vector<Vertex> vertVec = { vertices[0], vertices[1], vertices[2], vertices[3] };
        std::vector<unsigned int> indices = { 0, 1, 2, 0, 2, 3 };
        buffer.create(vertVec, indices);
        buffer.draw();
    }
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

void GLRenderer::showFps(bool show)
{
    _showFps = show;

    if (_window)
    {
        if (show)
        {
            // 保存当前窗口标题并初始化FPS计时器
            if (_windowTitle.empty())
            {
                const char* currentTitle = SDL_GetWindowTitle(_window);
                if (currentTitle)
                {
                    _windowTitle = currentTitle;
                }
            }
            _fpsLastTime = SDL_GetPerformanceCounter();
            _fpsFrameCount = 0;
        }
        else
        {
            // 恢复原始窗口标题
            if (!_windowTitle.empty())
            {
                SDL_SetWindowTitle(_window, _windowTitle.c_str());
            }
        }
    }
}

void GLRenderer::showBodyShapes(bool show)
{
    _showBodyShapes = show;
}

void GLRenderer::setVSync(bool enabled)
{
    _vSyncEnabled = enabled;
    if (_window)
    {
        SDL_GL_SetSwapInterval(enabled ? 1 : 0);
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
