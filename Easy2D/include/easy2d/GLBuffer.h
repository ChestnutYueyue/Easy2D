#pragma once
#include <easy2d/e2dmacros.h>
#include <glad/glad.h>
#include <easy2d/e2dmath.h>
#include <easy2d/e2dcommon.h>
#include <vector>

namespace easy2d
{

/**
 * @brief 顶点结构
 * 
 * 包含位置、纹理坐标和颜色信息
 */
struct Vertex
{
    Point position;     // 位置 (x, y)
    Point texCoord;     // 纹理坐标 (u, v)
    Color color;        // 颜色 (r, g, b, a)

    Vertex() = default;
    
    Vertex(const Point& pos, const Point& tex, const Color& col)
        : position(pos)
        , texCoord(tex)
        , color(col)
    {}
};

/**
 * @brief OpenGL顶点缓冲对象类
 * 
 * 管理VBO、VAO和EBO
 */
class GLBuffer
{
public:
    /**
     * @brief 构造函数
     */
    GLBuffer();

    /**
     * @brief 析构函数，自动释放资源
     */
    ~GLBuffer();

    /**
     * @brief 创建缓冲区
     * @param vertices 顶点数据
     * @param indices 索引数据（可选）
     * @param dynamic 是否动态更新
     * @return 是否创建成功
     */
    bool create(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices = {}, bool dynamic = false);

    /**
     * @brief 更新顶点数据
     * @param vertices 新的顶点数据
     */
    void updateVertices(const std::vector<Vertex>& vertices);

    /**
     * @brief 更新索引数据
     * @param indices 新的索引数据
     */
    void updateIndices(const std::vector<unsigned int>& indices);

    /**
     * @brief 绑定缓冲区
     */
    void bind() const;

    /**
     * @brief 解绑缓冲区
     */
    void unbind() const;

    /**
     * @brief 绘制
     * @param indexCount 要绘制的索引数量（0表示使用全部）
     */
    void draw(unsigned int indexCount = 0) const;

    /**
     * @brief 获取顶点数量
     */
    unsigned int getVertexCount() const { return _vertexCount; }

    /**
     * @brief 获取索引数量
     */
    unsigned int getIndexCount() const { return _indexCount; }

    /**
     * @brief 检查缓冲区是否有效
     */
    bool isValid() const { return _vao != 0; }

private:
    /**
     * @brief 释放资源
     */
    void release();

    /**
     * @brief 设置顶点属性指针
     */
    void setupVertexAttributes();

private:
    GLuint _vao;            // 顶点数组对象
    GLuint _vbo;            // 顶点缓冲对象
    GLuint _ebo;            // 元素缓冲对象
    unsigned int _vertexCount;  // 顶点数量
    unsigned int _indexCount;   // 索引数量
    bool _dynamic;          // 是否动态更新
};

/**
 * @brief 批次渲染器
 * 
 * 用于批量渲染多个精灵，减少绘制调用
 * 支持自动按纹理分组渲染
 */
class GLBatchRenderer
{
public:
    /**
     * @brief 构造函数
     * @param maxVertices 最大顶点数（默认4096）
     * @param maxIndices 最大索引数（默认6144）
     */
    GLBatchRenderer(unsigned int maxVertices = 4096, unsigned int maxIndices = 6144);

    /**
     * @brief 析构函数
     */
    ~GLBatchRenderer();

    /**
     * @brief 初始化批次渲染器
     * @return 是否初始化成功
     */
    bool initialize();

    /**
     * @brief 开始批次
     */
    void begin();

    /**
     * @brief 结束批次并提交绘制
     */
    void end();

    /**
     * @brief 添加四边形到批次
     * @param vertices 4个顶点
     * @param textureID 纹理ID（0表示无纹理）
     */
    void addQuad(const Vertex vertices[4], GLuint textureID = 0);

    /**
     * @brief 添加三角形到批次
     * @param vertices 3个顶点
     * @param textureID 纹理ID（0表示无纹理）
     */
    void addTriangle(const Vertex vertices[3], GLuint textureID = 0);

    /**
     * @brief 获取当前批次中的四边形数量
     */
    unsigned int getQuadCount() const { return _quadCount; }

    /**
     * @brief 获取当前批次中的纹理数量
     */
    unsigned int getTextureCount() const { return static_cast<unsigned int>(_textureBatches.size()); }

    /**
     * @brief 清空批次
     */
    void flush();

    /**
     * @brief 设置是否启用纹理图集模式
     * @param enabled 是否启用
     */
    void setAtlasMode(bool enabled) { _atlasMode = enabled; }

    /**
     * @brief 设置投影矩阵
     * @param projection 16个浮点数的投影矩阵
     */
    void setProjectionMatrix(const float* projection) { memcpy(_projectionMatrix, projection, sizeof(_projectionMatrix)); }

private:
    /**
     * @brief 纹理批次信息
     */
    struct TextureBatch
    {
        GLuint textureID;
        unsigned int startIndex;
        unsigned int indexCount;
        
        TextureBatch(GLuint id, unsigned int start)
            : textureID(id)
            , startIndex(start)
            , indexCount(0)
        {}
    };

    /**
     * @brief 提交当前批次到GPU
     */
    void submitBatch();

    /**
     * @brief 检查是否需要切换纹理批次
     * @param textureID 新纹理ID
     * @return 是否需要切换
     */
    bool needTextureSwitch(GLuint textureID) const;

    /**
     * @brief 开始新的纹理批次
     * @param textureID 纹理ID
     */
    void startNewTextureBatch(GLuint textureID);

private:
    // 批次数据
    std::vector<Vertex> _vertices;
    std::vector<unsigned int> _indices;
    std::vector<TextureBatch> _textureBatches;
    
    // OpenGL对象
    GLuint _vao;
    GLuint _vbo;
    GLuint _ebo;
    
    // 容量限制
    unsigned int _maxVertices;
    unsigned int _maxIndices;
    unsigned int _maxQuads;
    
    // 当前状态
    unsigned int _vertexCount;
    unsigned int _indexCount;
    unsigned int _quadCount;
    bool _batchStarted;
    GLuint _currentTextureID;
    bool _atlasMode;
    float _projectionMatrix[16];
};

} // namespace easy2d
