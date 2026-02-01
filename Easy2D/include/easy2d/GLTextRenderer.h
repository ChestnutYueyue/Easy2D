#pragma once
#include <easy2d/e2dcommon.h>
#include <easy2d/e2dtext.h>
#include <map>
#include <string>

struct FT_FaceRec_;
typedef struct FT_FaceRec_* FT_Face;
struct FT_LibraryRec_;
typedef struct FT_LibraryRec_* FT_Library;

namespace easy2d
{

class GLTexture;

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
	
private:
	GLTextRenderer();
	~GLTextRenderer();
	
	// 禁止拷贝
	GLTextRenderer(const GLTextRenderer&) = delete;
	GLTextRenderer& operator=(const GLTextRenderer&) = delete;
	
	// 创建字符纹理
	GLTexture* createGlyphTexture(FT_Face face, wchar_t charCode);
	
	// 生成字体键
	std::string getFontKey(const String& fontFamily, float fontSize, UINT fontWeight, wchar_t charCode);
	
private:
	FT_Library _ftLibrary;
	std::map<std::string, FT_Face> _fonts;
	std::map<std::string, GlyphInfo> _glyphs;
	bool _initialized;
};

// 便捷宏定义
#define E2D_GL_TEXT_RENDERER easy2d::GLTextRenderer::getInstance()

} // namespace easy2d