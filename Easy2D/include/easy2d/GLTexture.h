#pragma once
#include <easy2d/e2dmacros.h>
#include <glad/glad.h>
#include <easy2d/e2dmath.h>
#include <easy2d/e2dcommon.h>
#include <string>

namespace easy2d
{

/**
 * @brief OpenGL纹理类
 * 
 * 替代D2D1的ID2D1Bitmap，用于管理OpenGL纹理资源
 */
class GLTexture
{
public:
    /**
     * @brief 构造函数
     */
    GLTexture();

    /**
     * @brief 析构函数，自动释放纹理资源
     */
    ~GLTexture();

    /**
     * @brief 从文件加载纹理
     * @param filePath 图片文件路径
     * @return 是否加载成功
     */
    bool loadFromFile(const std::string& filePath);

    /**
     * @brief 从内存加载纹理
     * @param data 图片数据
     * @param size 数据大小
     * @return 是否加载成功
     */
    bool loadFromMemory(const unsigned char* data, size_t size);

    /**
	 * @brief 从原始像素数据创建纹理
	 * @param width 宽度
	 * @param height 高度
	 * @param data 像素数据（RGBA格式）
	 * @param generateMipmap 是否生成mipmap
	 * @return 是否创建成功
	 */
	bool createFromRawData(int width, int height, const unsigned char* data, bool generateMipmap = true);

	/**
	 * @brief 从原始像素数据创建字形纹理（使用NEAREST过滤保持边缘锐利）
	 * @param width 宽度
	 * @param height 高度
	 * @param data 像素数据（RGBA格式）
	 * @return 是否创建成功
	 */
	bool createFromRawDataForGlyph(int width, int height, const unsigned char* data);

    /**
     * @brief 创建空纹理（用于帧缓冲）
     * @param width 宽度
     * @param height 高度
     * @param format 纹理格式
     * @return 是否创建成功
     */
    bool createEmpty(int width, int height, GLenum format = GL_RGBA);

    /**
     * @brief 绑定纹理到指定纹理单元
     * @param unit 纹理单元（默认0）
     */
    void bind(unsigned int unit = 0) const;

    /**
     * @brief 解绑纹理
     */
    void unbind() const;

    /**
     * @brief 获取纹理ID
     * @return OpenGL纹理ID
     */
    GLuint getID() const { return _textureID; }

    /**
     * @brief 获取宽度
     */
    int getWidth() const { return _width; }

    /**
     * @brief 获取高度
     */
    int getHeight() const { return _height; }

    /**
     * @brief 获取大小
     */
    Size getSize() const { return Size(static_cast<float>(_width), static_cast<float>(_height)); }

    /**
     * @brief 设置过滤模式
     * @param minFilter 缩小过滤模式
     * @param magFilter 放大过滤模式
     */
    void setFilterMode(GLenum minFilter, GLenum magFilter);

    /**
     * @brief 设置环绕模式
     * @param wrapS S轴环绕模式
     * @param wrapT T轴环绕模式
     */
    void setWrapMode(GLenum wrapS, GLenum wrapT);

    /**
     * @brief 生成Mipmap
     */
    void generateMipmap();

    /**
     * @brief 检查纹理是否有效
     */
    bool isValid() const { return _textureID != 0; }

private:
    /**
     * @brief 释放纹理资源
     */
    void release();

private:
    GLuint _textureID;  // OpenGL纹理ID
    int _width;         // 纹理宽度
    int _height;        // 纹理高度
    GLenum _format;     // 纹理格式
};

/**
 * @brief 纹理缓存管理器
 * 
 * 管理纹理资源的缓存，避免重复加载
 */
class GLTextureCache
{
public:
    /**
     * @brief 获取单例实例
     */
    static GLTextureCache& getInstance();

    /**
     * @brief 从文件获取纹理（带缓存）
     * @param filePath 文件路径
     * @return 纹理对象，如果失败返回nullptr
     */
    GLTexture* getTexture(const std::string& filePath);

    /**
     * @brief 从资源获取纹理（带缓存）
     * @param res 资源对象
     * @return 纹理对象，如果失败返回nullptr
     */
    GLTexture* getTexture(const Resource& res);

    /**
     * @brief 添加纹理到缓存
     * @param key 缓存键
     * @param texture 纹理对象
     */
    void addTexture(const std::string& key, GLTexture* texture);

    /**
     * @brief 从缓存移除纹理
     * @param key 缓存键
     */
    void removeTexture(const std::string& key);

    /**
     * @brief 清空所有缓存
     */
    void clearCache();

    /**
     * @brief 重新加载所有缓存的纹理
     */
    void reloadAll();

private:
    GLTextureCache() = default;
    ~GLTextureCache();
    GLTextureCache(const GLTextureCache&) = delete;
    GLTextureCache& operator=(const GLTextureCache&) = delete;

private:
    std::unordered_map<std::string, GLTexture*> _textureMap;
};

// 便捷宏定义
#define E2D_GL_TEXTURE_CACHE easy2d::GLTextureCache::getInstance()

} // namespace easy2d
