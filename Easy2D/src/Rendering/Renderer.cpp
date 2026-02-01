#include <easy2d/e2dbase.h>
#include <easy2d/e2dmanager.h>
#include <easy2d/e2dnode.h>
#include <easy2d/e2dtext.h>
#include <easy2d/GLRenderer.h>
#include <easy2d/GLTexture.h>

namespace easy2d
{

// 文本渲染器适配（简化版本，使用FreeType替代DirectWrite）
class TextRenderer
{
public:
    static TextRenderer* Create()
    {
        return new (std::nothrow) TextRenderer();
    }

    void Prepare(GLRenderer* renderer, const Color& fillColor, bool hasOutline, 
                 const Color& outlineColor, float outlineWidth, LineJoin lineJoin)
    {
        _renderer = renderer;
        _fillColor = fillColor;
        _hasOutline = hasOutline;
        _outlineColor = outlineColor;
        _outlineWidth = outlineWidth;
        _lineJoin = lineJoin;
    }

private:
    GLRenderer* _renderer = nullptr;
    Color _fillColor;
    Color _outlineColor;
    float _outlineWidth = 1.0f;
    bool _hasOutline = false;
    LineJoin _lineJoin = LineJoin::None;
};

}

namespace
{
    bool s_bShowFps = false;
    bool s_bShowBodyShapes = false;
    bool s_bVSyncEnabled = true;
    bool s_bIsDeviceResourceRecreated = false;
    float s_fDpiScaleX = 96.0f;
    float s_fDpiScaleY = 96.0f;
    easy2d::Color s_nClearColor = easy2d::Color::Black;
    easy2d::TextRenderer* s_pTextRenderer = nullptr;
}

bool easy2d::Renderer::__createDeviceIndependentResources()
{
    // OpenGL不需要像D2D1那样创建设备无关资源
    // 所有资源都在GLRenderer::initialize中创建
    
    // 创建文本渲染器
    s_pTextRenderer = TextRenderer::Create();
    if (!s_pTextRenderer)
    {
        E2D_ERROR("Failed to create text renderer");
        return false;
    }

    return true;
}

bool easy2d::Renderer::__createDeviceResources()
{
    // 获取窗口句柄和尺寸
    HWND hWnd = Window::getHWnd();
    
    RECT rc;
    GetClientRect(hWnd, &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;

    // 获取DPI缩放
    HDC hdc = ::GetDC(hWnd);
    s_fDpiScaleX = (float)::GetDeviceCaps(hdc, LOGPIXELSX);
    s_fDpiScaleY = (float)::GetDeviceCaps(hdc, LOGPIXELSY);
    ::ReleaseDC(hWnd, hdc);

    // 初始化OpenGL渲染器
    if (!E2D_GL_RENDERER.initialize(hWnd, width, height))
    {
        E2D_ERROR("Failed to initialize GLRenderer");
        return false;
    }

    // 设置背景色
    E2D_GL_RENDERER.setBackgroundColor(s_nClearColor);

    // 设置垂直同步
    E2D_GL_RENDERER.setVSync(s_bVSyncEnabled);

    // 重新加载纹理缓存
    E2D_GL_TEXTURE_CACHE.reloadAll();

    s_bIsDeviceResourceRecreated = true;
    return true;
}

void easy2d::Renderer::__discardDeviceResources()
{
    // 关闭OpenGL渲染器
    E2D_GL_RENDERER.shutdown();
    s_bIsDeviceResourceRecreated = true;
}

void easy2d::Renderer::__discardResources()
{
    __discardDeviceResources();
    
    // 删除文本渲染器
    if (s_pTextRenderer)
    {
        delete s_pTextRenderer;
        s_pTextRenderer = nullptr;
    }

    // 清空纹理缓存
    E2D_GL_TEXTURE_CACHE.clearCache();
}

void easy2d::Renderer::__render()
{
    // 确保设备资源已创建
    if (!E2D_GL_RENDERER.getRenderTarget())
    {
        if (!__createDeviceResources())
        {
            E2D_ERROR("Failed to create device resources");
            return;
        }
    }

    // 开始一帧渲染
    E2D_GL_RENDERER.beginFrame();

    // 渲染场景
    SceneManager::__render(s_bShowBodyShapes);

    // 渲染自定义指针
    auto cursor = Window::getCustomCursor();
    if (cursor)
    {
        cursor->setPos(Input::getMousePos());
        cursor->_render();
    }

    // 渲染FPS
    if (s_bShowFps)
    {
        E2D_GL_RENDERER.showFps(true);
    }

    // 结束一帧渲染
    E2D_GL_RENDERER.endFrame();

    s_bIsDeviceResourceRecreated = false;
}

bool easy2d::Renderer::isDeviceResourceRecreated()
{
    return s_bIsDeviceResourceRecreated;
}

easy2d::Color easy2d::Renderer::getBackgroundColor()
{
    return s_nClearColor;
}

void easy2d::Renderer::setBackgroundColor(easy2d::Color color)
{
    s_nClearColor = color;
    E2D_GL_RENDERER.setBackgroundColor(color);
}

void easy2d::Renderer::showFps(bool show)
{
    s_bShowFps = show;
    E2D_GL_RENDERER.showFps(show);
}

void easy2d::Renderer::showBodyShapes(bool show)
{
    s_bShowBodyShapes = show;
    E2D_GL_RENDERER.showBodyShapes(show);
}

void easy2d::Renderer::setVSync(bool enabled)
{
    if (s_bVSyncEnabled != enabled)
    {
        s_bVSyncEnabled = enabled;
        E2D_GL_RENDERER.setVSync(enabled);
    }
}

bool easy2d::Renderer::isVSyncEnabled()
{
    return s_bVSyncEnabled;
}

float easy2d::Renderer::getDpiScaleX()
{
    return s_fDpiScaleX;
}

float easy2d::Renderer::getDpiScaleY()
{
    return s_fDpiScaleY;
}

// 以下D2D1特定接口已废弃，返回nullptr或默认值以兼容旧代码
void* easy2d::Renderer::getID2D1Factory()
{
    E2D_WARNING("getID2D1Factory is deprecated in OpenGL mode");
    return nullptr;
}

void* easy2d::Renderer::getRenderTarget()
{
    // 返回GLRenderer实例作为渲染目标标识
    return E2D_GL_RENDERER.getRenderTarget();
}

void* easy2d::Renderer::getSolidColorBrush()
{
    E2D_WARNING("getSolidColorBrush is deprecated in OpenGL mode");
    return nullptr;
}

void* easy2d::Renderer::getIWICImagingFactory()
{
    E2D_WARNING("getIWICImagingFactory is deprecated in OpenGL mode");
    return nullptr;
}

void* easy2d::Renderer::getIDWriteFactory()
{
    E2D_WARNING("getIDWriteFactory is deprecated in OpenGL mode");
    return nullptr;
}

void easy2d::Renderer::DrawTextLayout(TextLayout* layout, const DrawingStyle& style, const Point& offset, void* rt, void* brush)
{
    // 使用OpenGL渲染文本
    // TODO: 实现完整的文本渲染
    E2D_GL_RENDERER.drawTextLayout(layout, offset, style);
}

void* easy2d::Renderer::getMiterID2D1StrokeStyle()
{
    E2D_WARNING("getMiterID2D1StrokeStyle is deprecated in OpenGL mode");
    return nullptr;
}

void* easy2d::Renderer::getBevelID2D1StrokeStyle()
{
    E2D_WARNING("getBevelID2D1StrokeStyle is deprecated in OpenGL mode");
    return nullptr;
}

void* easy2d::Renderer::getRoundID2D1StrokeStyle()
{
    E2D_WARNING("getRoundID2D1StrokeStyle is deprecated in OpenGL mode");
    return nullptr;
}

// 获取GLRenderer实例（新增接口）
easy2d::GLRenderer* easy2d::Renderer::getGLRenderer()
{
    return &E2D_GL_RENDERER;
}
