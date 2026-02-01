#include <easy2d/GLTextRenderer.h>
#include <easy2d/GLRenderer.h>
#include <easy2d/GLTexture.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <easy2d/e2dtool.h>
#include <easy2d/e2dbase.h>

namespace easy2d
{

GLTextRenderer::GLTextRenderer()
	: _ftLibrary(nullptr)
	, _initialized(false)
{
}

GLTextRenderer::~GLTextRenderer()
{
	cleanup();
}

GLTextRenderer& GLTextRenderer::getInstance()
{
	static GLTextRenderer instance;
	return instance;
}

bool GLTextRenderer::init()
{
	if (_initialized)
	{
		return true;
	}
	
	// 初始化FreeType库
	if (FT_Init_FreeType(&_ftLibrary))
	{
		E2D_ERROR("Failed to initialize FreeType library");
		return false;
	}
	
	_initialized = true;
	return true;
}

void GLTextRenderer::cleanup()
{
	if (!_initialized)
	{
		return;
	}
	
	// 清理所有字形纹理
	for (auto& pair : _glyphs)
	{
		if (pair.second.texture)
		{
			delete pair.second.texture;
		}
	}
	_glyphs.clear();
	
	// 清理所有字体
	for (auto& pair : _fonts)
	{
		if (pair.second)
		{
			FT_Done_Face(pair.second);
		}
	}
	_fonts.clear();
	
	// 关闭FreeType库
	if (_ftLibrary)
	{
		FT_Done_FreeType(_ftLibrary);
		_ftLibrary = nullptr;
	}
	
	_initialized = false;
}

bool GLTextRenderer::loadFont(const String& family, const String& path)
{
	if (!_initialized)
	{
		if (!init())
		{
			return false;
		}
	}
	
	// 检查字体是否已加载
	if (_fonts.find(family) != _fonts.end())
	{
		return true;
	}
	
	// 加载字体文件
	FT_Face face;
	if (FT_New_Face(_ftLibrary, path.c_str(), 0, &face))
	{
		E2D_ERROR("Failed to load font: %s", path.c_str());
		return false;
	}
	
	_fonts[family] = face;
	return true;
}

GLTexture* GLTextRenderer::createGlyphTexture(FT_Face face, wchar_t charCode)
{
	// 加载字符字形，使用优化的加载标志提高渲染质量
	// FT_LOAD_TARGET_LIGHT: 使用轻量级抗锯齿，适合小字号
	// FT_LOAD_RENDER: 立即渲染到位图
	// FT_LOAD_FORCE_AUTOHINT: 强制使用自动提示，提高清晰度
	int loadFlags = FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT;
	if (FT_Load_Char(face, charCode, loadFlags))
	{
		return nullptr;
	}
	
	// 获取字形位图
	FT_GlyphSlot glyph = face->glyph;
	unsigned int width = glyph->bitmap.width;
	unsigned int height = glyph->bitmap.rows;
	unsigned char* buffer = glyph->bitmap.buffer;
	
	if (width == 0 || height == 0 || buffer == nullptr)
	{
		return nullptr;
	}
	
	// 创建GLTexture对象
	GLTexture* texture = new GLTexture();
	
	// 从原始数据创建纹理（使用GL_RED作为单通道）
	// 需要将单通道数据转换为RGBA格式
	// 注意：FreeType位图的每一行可能有填充，需要使用pitch而不是width
	int pitch = glyph->bitmap.pitch;
	std::vector<unsigned char> rgbaData(width * height * 4);
	for (unsigned int y = 0; y < height; ++y)
	{
		for (unsigned int x = 0; x < width; ++x)
		{
			unsigned char alpha = buffer[y * pitch + x];
			unsigned int index = (y * width + x) * 4;
			rgbaData[index] = 255;     // R
			rgbaData[index + 1] = 255; // G
			rgbaData[index + 2] = 255; // B
			rgbaData[index + 3] = alpha; // A
		}
	}
	
	// 使用专门的方法创建字形纹理，使用NEAREST过滤保持边缘锐利
	texture->createFromRawDataForGlyph(static_cast<int>(width), static_cast<int>(height), rgbaData.data());
	
	return texture;
}

std::string GLTextRenderer::getFontKey(const String& fontFamily, float fontSize, UINT fontWeight, wchar_t charCode)
{
	char key[256];
	// 使用整数字体大小避免浮点数精度问题
	// 使用 %u 将 wchar_t 作为无符号整数输出，避免 %lc 的编码问题
	snprintf(key, sizeof(key), "%s_%d_%u_%u", fontFamily.c_str(), static_cast<int>(fontSize), fontWeight, static_cast<unsigned int>(charCode));
	return std::string(key);
}

GlyphInfo* GLTextRenderer::getGlyph(const String& fontFamily, wchar_t charCode, float fontSize, UINT fontWeight)
{
	std::string key = getFontKey(fontFamily, fontSize, fontWeight, charCode);
	
	// 检查缓存
	auto it = _glyphs.find(key);
	if (it != _glyphs.end())
	{
		return &it->second;
	}
	
	// 查找字体
	auto fontIt = _fonts.find(fontFamily);
	if (fontIt == _fonts.end())
	{
		// 尝试加载默认字体
		if (!loadFont(fontFamily, "C:\\Windows\\Fonts\\msyh.ttc"))
		{
			return nullptr;
		}
		fontIt = _fonts.find(fontFamily);
		if (fontIt == _fonts.end())
		{
			return nullptr;
		}
	}
	
	FT_Face face = fontIt->second;
	
	// 设置字体大小
	FT_Set_Pixel_Sizes(face, 0, static_cast<FT_UInt>(fontSize));
	
	// 应用字体粗细（通过调整字形轮廓）
	// 注意：FreeType的字体粗细支持有限，这里使用简化实现
	// 实际应用中可能需要使用支持多字重的字体文件
	
	// 创建字符纹理
	GLTexture* texture = createGlyphTexture(face, charCode);
	if (!texture)
	{
		return nullptr;
	}
	
	// 创建字形信息
	GlyphInfo info;
	info.texture = texture;
	info.advanceX = static_cast<float>(face->glyph->advance.x >> 6);
	info.advanceY = static_cast<float>(face->glyph->advance.y >> 6);
	info.bearingX = static_cast<float>(face->glyph->bitmap_left);
	info.bearingY = static_cast<float>(face->glyph->bitmap_top);
	info.width = static_cast<float>(face->glyph->bitmap.width);
	info.height = static_cast<float>(face->glyph->bitmap.rows);
	
	// 缓存字形信息
	_glyphs[key] = info;
	
	return &_glyphs[key];
}

Size GLTextRenderer::calculateTextSize(const String& text, const TextStyle& style)
{
	if (text.empty())
	{
		return Size(0, 0);
	}
	
	WideString wideText = NarrowToWide(text);
	float fontSize = style.font.size > 0 ? style.font.size : 12.0f;
	
	float x = 0;
	float y = 0;
	float maxWidth = 0;
	float lineHeight = fontSize + style.lineSpacing;
	
	for (size_t i = 0; i < wideText.length(); ++i)
	{
		wchar_t c = wideText[i];
		
		if (c == L'\n')
		{
			// 换行
			x = 0;
			y += lineHeight;
			continue;
		}
		
		GlyphInfo* glyph = getGlyph(style.font.family, c, fontSize, style.font.weight);
		if (glyph)
		{
			x += glyph->advanceX;
			
			// 检查是否需要换行
			if (style.wrapping && style.wrappingWidth > 0 && x > style.wrappingWidth)
			{
				x = glyph->advanceX;
				y += lineHeight;
			}
			
			if (x > maxWidth)
			{
				maxWidth = x;
			}
		}
	}
	
	return Size(maxWidth, y + lineHeight);
}

void GLTextRenderer::renderText(const String& text, const Point& pos, const TextStyle& style, const Color& color)
{
	if (text.empty())
	{
		return;
	}
	
	WideString wideText = NarrowToWide(text);
	float fontSize = style.font.size > 0 ? style.font.size : 12.0f;
	
	GLRenderer* glRenderer = Renderer::getGLRenderer();
	if (!glRenderer)
	{
		return;
	}
	
	float lineHeight = fontSize + style.lineSpacing;
	
	// 计算文本总宽度
	Size textSize = calculateTextSize(text, style);
	float textWidth = textSize.width;
	
	// 根据对齐方式计算起始X位置
	float startX = pos.x;
	switch (style.alignment)
	{
	case TextAlign::Center:
		startX = pos.x - textWidth * 0.5f;
		break;
	case TextAlign::Right:
		startX = pos.x - textWidth;
		break;
	case TextAlign::Left:
	default:
		startX = pos.x;
		break;
	}
	
	float x = startX;
	float y = pos.y;
	
	// 处理自动换行
	if (style.wrapping && style.wrappingWidth > 0)
	{
		float lineWidth = 0;
		float lineStartX = x;
		for (size_t i = 0; i < wideText.length(); ++i)
		{
			wchar_t c = wideText[i];
			
			if (c == L'\n')
			{
				// 渲染当前行的下划线和删除线
				if (style.hasUnderline)
				{
					float underlineY = y + fontSize * 0.1f;
					Rect underlineRect(Point(lineStartX, underlineY), Size(lineWidth, 1.0f));
					glRenderer->drawRect(underlineRect, color);
				}
				
				if (style.hasStrikethrough)
				{
					float strikeY = y - fontSize * 0.3f;
					Rect strikeRect(Point(lineStartX, strikeY), Size(lineWidth, 1.0f));
					glRenderer->drawRect(strikeRect, color);
				}
				
				// 换行
				lineWidth = 0;
				lineStartX = startX;
				x = startX;
				y += lineHeight;
				continue;
			}
			
			GlyphInfo* glyph = getGlyph(style.font.family, c, fontSize, style.font.weight);
			if (glyph && glyph->texture)
			{
				// 检查是否需要换行
				if (lineWidth + glyph->advanceX > style.wrappingWidth)
				{
					// 渲染当前行的下划线和删除线
					if (style.hasUnderline)
					{
						float underlineY = y + fontSize * 0.1f;
						Rect underlineRect(Point(lineStartX, underlineY), Size(lineWidth, 1.0f));
						glRenderer->drawRect(underlineRect, color);
					}
					
					if (style.hasStrikethrough)
					{
						float strikeY = y - fontSize * 0.3f;
						Rect strikeRect(Point(lineStartX, strikeY), Size(lineWidth, 1.0f));
						glRenderer->drawRect(strikeRect, color);
					}
					
					lineWidth = glyph->advanceX;
					lineStartX = x;
					x = startX;
					y += lineHeight;
				}
				else
				{
					lineWidth += glyph->advanceX;
				}
				
				// 计算字符位置
				float xPos = x + glyph->bearingX;
				float yPos = y - glyph->bearingY;
				
				// 渲染字符
				Rect destRect(Point(xPos, yPos), Size(glyph->width, glyph->height));
				glRenderer->setOpacity(color.a);
				glRenderer->drawTexture(glyph->texture, destRect, nullptr, color);
				
				// 移动到下一个字符位置
				x += glyph->advanceX;
			}
		}
		
		// 渲染最后一行的下划线和删除线
		if (style.hasUnderline)
		{
			float underlineY = y + fontSize * 0.1f;
			Rect underlineRect(Point(lineStartX, underlineY), Size(lineWidth, 1.0f));
			glRenderer->drawRect(underlineRect, color);
		}
		
		if (style.hasStrikethrough)
		{
			float strikeY = y - fontSize * 0.3f;
			Rect strikeRect(Point(lineStartX, strikeY), Size(lineWidth, 1.0f));
			glRenderer->drawRect(strikeRect, color);
		}
	}
	else
	{
		// 不换行，直接渲染
		float lineStartX = x;
		for (size_t i = 0; i < wideText.length(); ++i)
		{
			wchar_t c = wideText[i];
			
			if (c == L'\n')
			{
				// 渲染当前行的下划线和删除线
				if (style.hasUnderline)
				{
					float underlineY = y + fontSize * 0.1f;
					Rect underlineRect(Point(lineStartX, underlineY), Size(x - lineStartX, 1.0f));
					glRenderer->drawRect(underlineRect, color);
				}
				
				if (style.hasStrikethrough)
				{
					float strikeY = y - fontSize * 0.3f;
					Rect strikeRect(Point(lineStartX, strikeY), Size(x - lineStartX, 1.0f));
					glRenderer->drawRect(strikeRect, color);
				}
				
				// 换行
				lineStartX = startX;
				x = startX;
				y += lineHeight;
				continue;
			}
			
			GlyphInfo* glyph = getGlyph(style.font.family, c, fontSize, style.font.weight);
			if (glyph && glyph->texture)
			{
				// 计算字符位置
				float xPos = x + glyph->bearingX;
				float yPos = y - glyph->bearingY;
				
				// 渲染字符
				Rect destRect(Point(xPos, yPos), Size(glyph->width, glyph->height));
				glRenderer->setOpacity(color.a);
				glRenderer->drawTexture(glyph->texture, destRect, nullptr, color);
				
				// 移动到下一个字符位置
				x += glyph->advanceX;
			}
		}
		
		// 渲染最后一行的下划线和删除线
		if (style.hasUnderline)
		{
			float underlineY = y + fontSize * 0.1f;
			Rect underlineRect(Point(lineStartX, underlineY), Size(x - lineStartX, 1.0f));
			glRenderer->drawRect(underlineRect, color);
		}
		
		if (style.hasStrikethrough)
		{
			float strikeY = y - fontSize * 0.3f;
			Rect strikeRect(Point(lineStartX, strikeY), Size(x - lineStartX, 1.0f));
			glRenderer->drawRect(strikeRect, color);
		}
	}
}

}
