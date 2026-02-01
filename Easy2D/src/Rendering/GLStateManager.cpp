#include <easy2d/GLStateManager.h>
#include <easy2d/e2dbase.h>

namespace easy2d
{

GLStateManager& GLStateManager::getInstance()
{
    static GLStateManager instance;
    return instance;
}

void GLStateManager::initialize()
{
    // 初始化默认状态
    reset();
    E2D_LOG("GLStateManager initialized");
}

void GLStateManager::reset()
{
    // 重置所有状态为默认值
    setBlend(true);
    setBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    setBlendEquation(GL_FUNC_ADD);
    
    setDepthTest(false);
    setDepthFunc(GL_LESS);
    setDepthWrite(true);
    
    setStencilTest(false);
    setStencilFunc(GL_ALWAYS, 0, 0xFFFFFFFF);
    setStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    
    setCullFace(false);
    setCullFaceMode(GL_BACK);
    
    setScissorTest(false);
    setScissorRect(0, 0, 0, 0);
    
    setWireframe(false);
}

void GLStateManager::setBlend(bool enabled)
{
    if (_blendEnabled != enabled)
    {
        _blendEnabled = enabled;
        if (enabled)
        {
            glEnable(GL_BLEND);
        }
        else
        {
            glDisable(GL_BLEND);
        }
    }
}

void GLStateManager::setBlendFunc(GLenum srcFactor, GLenum dstFactor)
{
    if (_blendSrcFactor != srcFactor || _blendDstFactor != dstFactor)
    {
        _blendSrcFactor = srcFactor;
        _blendDstFactor = dstFactor;
        glBlendFunc(srcFactor, dstFactor);
    }
}

void GLStateManager::setBlendEquation(GLenum mode)
{
    if (_blendEquation != mode)
    {
        _blendEquation = mode;
        glBlendEquation(mode);
    }
}

void GLStateManager::setDepthTest(bool enabled)
{
    if (_depthTestEnabled != enabled)
    {
        _depthTestEnabled = enabled;
        if (enabled)
        {
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }
    }
}

void GLStateManager::setDepthFunc(GLenum func)
{
    if (_depthFunc != func)
    {
        _depthFunc = func;
        glDepthFunc(func);
    }
}

void GLStateManager::setDepthWrite(bool enabled)
{
    if (_depthWriteEnabled != enabled)
    {
        _depthWriteEnabled = enabled;
        glDepthMask(enabled ? GL_TRUE : GL_FALSE);
    }
}

void GLStateManager::setStencilTest(bool enabled)
{
    if (_stencilTestEnabled != enabled)
    {
        _stencilTestEnabled = enabled;
        if (enabled)
        {
            glEnable(GL_STENCIL_TEST);
        }
        else
        {
            glDisable(GL_STENCIL_TEST);
        }
    }
}

void GLStateManager::setStencilFunc(GLenum func, GLint ref, GLuint mask)
{
    if (_stencilFunc != func || _stencilRef != ref || _stencilMask != mask)
    {
        _stencilFunc = func;
        _stencilRef = ref;
        _stencilMask = mask;
        glStencilFunc(func, ref, mask);
    }
}

void GLStateManager::setStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass)
{
    if (_stencilSFail != sfail || _stencilDPFail != dpfail || _stencilDPPass != dppass)
    {
        _stencilSFail = sfail;
        _stencilDPFail = dpfail;
        _stencilDPPass = dppass;
        glStencilOp(sfail, dpfail, dppass);
    }
}

void GLStateManager::setCullFace(bool enabled)
{
    if (_cullFaceEnabled != enabled)
    {
        _cullFaceEnabled = enabled;
        if (enabled)
        {
            glEnable(GL_CULL_FACE);
        }
        else
        {
            glDisable(GL_CULL_FACE);
        }
    }
}

void GLStateManager::setCullFaceMode(GLenum mode)
{
    if (_cullFaceMode != mode)
    {
        _cullFaceMode = mode;
        glCullFace(mode);
    }
}

void GLStateManager::setScissorTest(bool enabled)
{
    if (_scissorTestEnabled != enabled)
    {
        _scissorTestEnabled = enabled;
        if (enabled)
        {
            glEnable(GL_SCISSOR_TEST);
        }
        else
        {
            glDisable(GL_SCISSOR_TEST);
        }
    }
}

void GLStateManager::setScissorRect(int x, int y, int width, int height)
{
    if (_scissorX != x || _scissorY != y || 
        _scissorWidth != width || _scissorHeight != height)
    {
        _scissorX = x;
        _scissorY = y;
        _scissorWidth = width;
        _scissorHeight = height;
        glScissor(x, y, width, height);
    }
}

void GLStateManager::setWireframe(bool enabled)
{
    if (_wireframeEnabled != enabled)
    {
        _wireframeEnabled = enabled;
        glPolygonMode(GL_FRONT_AND_BACK, enabled ? GL_LINE : GL_FILL);
    }
}

void GLStateManager::setViewport(int x, int y, int width, int height)
{
    if (_viewportX != x || _viewportY != y || 
        _viewportWidth != width || _viewportHeight != height)
    {
        _viewportX = x;
        _viewportY = y;
        _viewportWidth = width;
        _viewportHeight = height;
        glViewport(x, y, width, height);
    }
}

void GLStateManager::pushState()
{
    StateSnapshot snapshot;
    snapshot.blendEnabled = _blendEnabled;
    snapshot.blendSrcFactor = _blendSrcFactor;
    snapshot.blendDstFactor = _blendDstFactor;
    snapshot.depthTestEnabled = _depthTestEnabled;
    snapshot.scissorTestEnabled = _scissorTestEnabled;
    snapshot.scissorX = _scissorX;
    snapshot.scissorY = _scissorY;
    snapshot.scissorWidth = _scissorWidth;
    snapshot.scissorHeight = _scissorHeight;
    snapshot.wireframeEnabled = _wireframeEnabled;
    
    _stateStack.push(snapshot);
}

void GLStateManager::popState()
{
    if (_stateStack.empty())
    {
        E2D_WARNING("GLStateManager::popState() called with empty stack");
        return;
    }
    
    StateSnapshot snapshot = _stateStack.top();
    _stateStack.pop();
    
    // 恢复状态
    setBlend(snapshot.blendEnabled);
    setBlendFunc(snapshot.blendSrcFactor, snapshot.blendDstFactor);
    setDepthTest(snapshot.depthTestEnabled);
    setScissorTest(snapshot.scissorTestEnabled);
    if (snapshot.scissorTestEnabled)
    {
        setScissorRect(snapshot.scissorX, snapshot.scissorY, 
                       snapshot.scissorWidth, snapshot.scissorHeight);
    }
    setWireframe(snapshot.wireframeEnabled);
}

} // namespace easy2d
