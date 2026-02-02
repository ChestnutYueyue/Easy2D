#pragma once
#include <easy2d/e2dcommon.h>
#include <easy2d/e2dtext.h>
#include <map>
#include <string>
#include <list>
#include <memory>
#include <glad/glad.h>

struct FT_FaceRec_;
typedef struct FT_FaceRec_* FT_Face;
struct FT_LibraryRec_;
typedef struct FT_LibraryRec_* FT_Library;

namespace easy2d
{

class GLTexture;
class GLBatchRenderer;

// 文本字符信息
struct GlyphInfo
{
	GLTexture* texture;
	float advanceX;
	float advanceY;
	float bearingX;
	float bearingY;
	float width;
	float height;
	
	GlyphInfo()
		: texture(nullptr)
		, advanceX(0)
		, advanceY(0)
		, bearingX(0)
		, bearingY(0)
		, width(0)
		, height(0)
	{
	}
};

// LRU缓存节点
struct GlyphCacheNode
{
	std::string key;
	GlyphInfo info;
	
	GlyphCacheNode(const std::string& k, const GlyphInfo& i)
		: key(k)
		, info(i)
	{
	}
};

// OpenGL文本渲染器
class GLTextRenderer
{
public:
	static GLTextRenderer& getInstance();
	
	// 初始化FreeType库
	bool init();
	
	// 清理资源
	void cleanup();
	
	// 加载字体
	bool loadFont(const String& family, const String& path);
	
	// 获取字符字形信息
	GlyphInfo* getGlyph(const String& fontFamily, wchar_t charCode, float fontSize, UINT fontWeight = Font::Weight::Normal);
	
	// 计算文本尺寸
	Size calculateTextSize(const String& text, const TextStyle& style);
	
	// 渲染文本
	void renderText(const String& text, const Point& pos, const TextStyle& style, const Color& color);
	
	// 获取缓存统计信息
	void getCacheStats(int& totalGlyphs, int& cacheHits, int& cacheMisses) const;
	
	// 清空字形缓存
	void clearGlyphCache();

private:
	GLTextRenderer();
	~GLTextRenderer();
	
	// 禁止拷贝
	GLTextRenderer(const GLTextRenderer&) = delete;
	GLTextRenderer& operator=(const GLTextRenderer&) = delete;
	
	// 创建字符纹理
	GLTexture* createGlyphTexture(FT_Face face, wchar_t charCode, float fontSize);
	
	// 生成字体键
	std::string getFontKey(const String& fontFamily, float fontSize, UINT fontWeight, wchar_t charCode);
	
	// 根据字号选择纹理过滤模式
	GLenum getFilterModeForFontSize(float fontSize) const;
	
	// 更新LRU缓存
	void updateLRU(const std::string& key, const GlyphInfo& info);
	
	// 检查并清理缓存（如果超过限制）
	void checkCacheLimit();
	
	// 渲染下划线或删除线
	void renderDecorationLine(const Point& startPos, float lineWidth, float yPos, const Color& color, float fontSize, bool isUnderline);

private:
	FT_Library _ftLibrary;
	std::map<std::string, FT_Face> _fonts;
	
	// LRU缓存
	std::list<GlyphCacheNode> _lruList;
	std::map<std::string, std::list<GlyphCacheNode>::iterator> _lruMap;
	size_t _maxCacheSize;
	
	// 统计信息
	int _cacheHits;
	int _cacheMisses;
	
	bool _initialized;
};

// 便捷宏定义
#define E2D_GL_TEXT_RENDERER easy2d::GLTextRenderer::getInstance()

} // namespace easy2d