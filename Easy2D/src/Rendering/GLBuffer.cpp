#include <easy2d/GLBuffer.h>
#include <easy2d/e2dbase.h>

namespace easy2d
{

// GLBuffer 实现

GLBuffer::GLBuffer()
    : _vao(0)
    , _vbo(0)
    , _ebo(0)
    , _vertexCount(0)
    , _indexCount(0)
    , _dynamic(false)
{
}

GLBuffer::~GLBuffer()
{
    release();
}

void GLBuffer::release()
{
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
    if (_ebo != 0)
    {
        glDeleteBuffers(1, &_ebo);
        _ebo = 0;
    }
    _vertexCount = 0;
    _indexCount = 0;
}

bool GLBuffer::create(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, bool dynamic)
{
    release();

    _dynamic = dynamic;
    _vertexCount = static_cast<unsigned int>(vertices.size());
    _indexCount = static_cast<unsigned int>(indices.size());

    // 创建VAO
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    // 创建VBO
    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    
    GLenum usage = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), usage);

    // 创建EBO（如果有索引）
    if (!indices.empty())
    {
        glGenBuffers(1, &_ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), usage);
    }

    // 设置顶点属性
    setupVertexAttributes();

    // 解绑
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (_ebo != 0)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    return true;
}

void GLBuffer::setupVertexAttributes()
{
    // 位置属性 (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    // 纹理坐标属性 (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    // 颜色属性 (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
}

void GLBuffer::updateVertices(const std::vector<Vertex>& vertices)
{
    if (!_dynamic)
    {
        E2D_WARNING("Trying to update non-dynamic buffer");
        return;
    }

    _vertexCount = static_cast<unsigned int>(vertices.size());
    
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLBuffer::updateIndices(const std::vector<unsigned int>& indices)
{
    if (!_dynamic || _ebo == 0)
    {
        E2D_WARNING("Trying to update non-dynamic buffer or buffer without indices");
        return;
    }

    _indexCount = static_cast<unsigned int>(indices.size());
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(unsigned int), indices.data());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLBuffer::bind() const
{
    glBindVertexArray(_vao);
}

void GLBuffer::unbind() const
{
    glBindVertexArray(0);
}

void GLBuffer::draw(unsigned int indexCount) const
{
    bind();
    
    unsigned int count = (indexCount == 0) ? _indexCount : indexCount;
    
    if (count > 0)
    {
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, _vertexCount);
    }
    
    unbind();
}

// GLBatchRenderer 实现

GLBatchRenderer::GLBatchRenderer(unsigned int maxVertices, unsigned int maxIndices)
    : _vao(0)
    , _vbo(0)
    , _ebo(0)
    , _maxVertices(maxVertices)
    , _maxIndices(maxIndices)
    , _maxQuads(maxVertices / 4)
    , _vertexCount(0)
    , _indexCount(0)
    , _quadCount(0)
    , _batchStarted(false)
{
    // 预分配空间
    _vertices.reserve(maxVertices);
    _indices.reserve(maxIndices);
}

GLBatchRenderer::~GLBatchRenderer()
{
    if (_vao != 0)
    {
        glDeleteVertexArrays(1, &_vao);
    }
    if (_vbo != 0)
    {
        glDeleteBuffers(1, &_vbo);
    }
    if (_ebo != 0)
    {
        glDeleteBuffers(1, &_ebo);
    }
}

bool GLBatchRenderer::initialize()
{
    // 创建VAO
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    // 创建VBO
    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, _maxVertices * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

    // 创建EBO
    glGenBuffers(1, &_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _maxIndices * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);

    // 设置顶点属性
    // 位置属性 (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    // 纹理坐标属性 (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    // 颜色属性 (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

    // 解绑
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return true;
}

void GLBatchRenderer::begin()
{
    _batchStarted = true;
    _vertexCount = 0;
    _indexCount = 0;
    _quadCount = 0;
    _vertices.clear();
    _indices.clear();
}

void GLBatchRenderer::end()
{
    if (!_batchStarted)
    {
        return;
    }

    submitBatch();
    _batchStarted = false;
}

void GLBatchRenderer::addQuad(const Vertex vertices[4], GLuint textureID)
{
    if (!_batchStarted)
    {
        E2D_WARNING("Batch not started");
        return;
    }

    // 检查是否需要提交当前批次
    if (_quadCount >= _maxQuads)
    {
        submitBatch();
        begin();
    }

    // 添加顶点
    unsigned int baseIndex = _vertexCount;
    for (int i = 0; i < 4; ++i)
    {
        _vertices.push_back(vertices[i]);
    }

    // 添加索引（两个三角形组成一个四边形）
    _indices.push_back(baseIndex);
    _indices.push_back(baseIndex + 1);
    _indices.push_back(baseIndex + 2);
    _indices.push_back(baseIndex);
    _indices.push_back(baseIndex + 2);
    _indices.push_back(baseIndex + 3);

    _vertexCount += 4;
    _indexCount += 6;
    _quadCount++;
}

void GLBatchRenderer::addTriangle(const Vertex vertices[3], GLuint textureID)
{
    if (!_batchStarted)
    {
        E2D_WARNING("Batch not started");
        return;
    }

    // 检查是否需要提交当前批次
    if (_vertexCount + 3 > _maxVertices)
    {
        submitBatch();
        begin();
    }

    // 添加顶点
    unsigned int baseIndex = _vertexCount;
    for (int i = 0; i < 3; ++i)
    {
        _vertices.push_back(vertices[i]);
    }

    // 添加索引
    _indices.push_back(baseIndex);
    _indices.push_back(baseIndex + 1);
    _indices.push_back(baseIndex + 2);

    _vertexCount += 3;
    _indexCount += 3;
}

void GLBatchRenderer::flush()
{
    if (_vertexCount > 0)
    {
        submitBatch();
    }
}

void GLBatchRenderer::submitBatch()
{
    if (_vertexCount == 0)
    {
        return;
    }

    // 绑定VAO
    glBindVertexArray(_vao);

    // 更新VBO数据
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, _vertices.size() * sizeof(Vertex), _vertices.data());

    // 更新EBO数据
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, _indices.size() * sizeof(unsigned int), _indices.data());

    // 绘制
    glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_INT, nullptr);

    // 解绑
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // 重置计数
    _vertexCount = 0;
    _indexCount = 0;
    _quadCount = 0;
    _vertices.clear();
    _indices.clear();
}

} // namespace easy2d
