#include <easy2d/GLFullScreenQuad.h>
#include <easy2d/e2dbase.h>

namespace easy2d
{

// 全屏四边形顶点数据（位置和纹理坐标）
static float quadVertices[] = {
    // 位置          // 纹理坐标
    -1.0f,  1.0f,   0.0f, 1.0f,  // 左上
    -1.0f, -1.0f,   0.0f, 0.0f,  // 左下
     1.0f, -1.0f,   1.0f, 0.0f,  // 右下
    -1.0f,  1.0f,   0.0f, 1.0f,  // 左上
     1.0f, -1.0f,   1.0f, 0.0f,  // 右下
     1.0f,  1.0f,   1.0f, 1.0f   // 右上
};

GLFullScreenQuad& GLFullScreenQuad::getInstance()
{
    static GLFullScreenQuad instance;
    return instance;
}

bool GLFullScreenQuad::initialize()
{
    if (_initialized)
    {
        return true;
    }

    // 创建VAO
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    // 位置属性 (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    
    // 纹理坐标属性 (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    _initialized = true;
    E2D_LOG("GLFullScreenQuad initialized");
    return true;
}

void GLFullScreenQuad::shutdown()
{
    if (!_initialized)
    {
        return;
    }

    if (_vao != 0)
    {
        glDeleteVertexArrays(1, &_vao);
        _vao = 0;
    }
    if (_vbo != 0)
    {
        glDeleteBuffers(1, &_vbo);
        _vbo = 0;
    }

    _initialized = false;
    E2D_LOG("GLFullScreenQuad shutdown");
}

void GLFullScreenQuad::render() const
{
    if (!_initialized)
    {
        return;
    }

    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

} // namespace easy2d
