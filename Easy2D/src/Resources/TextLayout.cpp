#include <easy2d/e2dtext.h>
#include <easy2d/e2dbase.h>
#include <easy2d/GLTextRenderer.h>

easy2d::TextLayout::TextLayout()
	: _size()
	, _text()
	, _style()
{
}

easy2d::TextLayout::TextLayout(const String& text, const TextStyle& style)
	: _size()
	, _text(text)
	, _style(style)
{
	_recreateLayout();
}

easy2d::TextLayout::~TextLayout()
{
}

const easy2d::String& easy2d::TextLayout::getText() const
{
	return _text;
}

easy2d::Font easy2d::TextLayout::getFont() const
{
	return _style.font;
}

easy2d::TextStyle easy2d::TextLayout::getStyle() const
{
	return _style;
}

int easy2d::TextLayout::getLineCount() const
{
	if (_text.empty())
	{
		return 0;
	}
	
	int lineCount = 1;
	for (char c : _text)
	{
		if (c == '\n')
		{
			lineCount++;
		}
	}
	
	return lineCount;
}

bool easy2d::TextLayout::hasStrikethrough() const
{
	return _style.hasStrikethrough;
}

bool easy2d::TextLayout::hasUnderline() const
{
	return _style.hasUnderline;
}

easy2d::Size easy2d::TextLayout::getSize() const
{
	return _size;
}

void easy2d::TextLayout::setText(const String& text)
{
	if (_text != text)
	{
		_text = text;
		_recreateLayout();
	}
}

void easy2d::TextLayout::setStyle(const TextStyle& style)
{
	_style = style;
	_recreateLayout();
}

void easy2d::TextLayout::setFont(const Font& font)
{
	_style.font = font;
	_recreateLayout();
}

void easy2d::TextLayout::setFontFamily(const String& family)
{
	if (_style.font.family != family)
	{
		_style.font.family = family;
		_recreateLayout();
	}
}

void easy2d::TextLayout::setFontSize(float size)
{
	if (_style.font.size != size)
	{
		_style.font.size = size;
		_recreateLayout();
	}
}

void easy2d::TextLayout::setFontWeight(UINT weight)
{
	if (_style.font.weight != weight)
	{
		_style.font.weight = weight;
		_recreateLayout();
	}
}

void easy2d::TextLayout::setItalic(bool italic)
{
	if (_style.font.italic != italic)
	{
		_style.font.italic = italic;
		_recreateLayout();
	}
}

void easy2d::TextLayout::setWrapping(bool wrapping)
{
	if (_style.wrapping != wrapping)
	{
		_style.wrapping = wrapping;
		_recreateLayout();
	}
}

void easy2d::TextLayout::setWrappingWidth(float wrappingWidth)
{
	if (_style.wrappingWidth != wrappingWidth)
	{
		_style.wrappingWidth = (wrappingWidth > 0.0f) ? wrappingWidth : 0.0f;
		
		if (_style.wrapping)
		{
			_recreateLayout();
		}
	}
}

void easy2d::TextLayout::setLineSpacing(float lineSpacing)
{
	if (_style.lineSpacing != lineSpacing)
	{
		_style.lineSpacing = lineSpacing;
		_recreateLayout();
	}
}

void easy2d::TextLayout::setAlignment(TextAlign align)
{
	if (_style.alignment != align)
	{
		_style.alignment = align;
		_recreateLayout();
	}
}

void easy2d::TextLayout::setUnderline(bool hasUnderline)
{
	if (_style.hasUnderline != hasUnderline)
	{
		_style.hasUnderline = hasUnderline;
		_recreateLayout();
	}
}

void easy2d::TextLayout::setStrikethrough(bool hasStrikethrough)
{
	if (_style.hasStrikethrough != hasStrikethrough)
	{
		_style.hasStrikethrough = hasStrikethrough;
		_recreateLayout();
	}
}

void easy2d::TextLayout::reset(const String& text, const TextStyle& style)
{
	_text = text;
	_style = style;
	_recreateLayout();
}

void easy2d::TextLayout::_recreateLayout()
{
	if (_text.empty())
	{
		_size = Size{};
		return;
	}
	// 使用GLTextRenderer计算文本尺寸
	GLTextRenderer& textRenderer = GLTextRenderer::getInstance();
	_size = textRenderer.calculateTextSize(_text, _style);
}
