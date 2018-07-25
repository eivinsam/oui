#include "oui_text_internal.h"

#include <stdexcept>
#include <memory>
#include <array>


namespace oui
{
	using uchar = unsigned char;

	int popCodepoint(std::string_view& text)
	{
		if (text.empty())
			return 0;
		int code = 0;
		const uchar x = uchar(text.front()); text.remove_prefix(1);
		uchar remaining = 0;
		switch (x >> 4)
		{
		case 0b1111:
			code = x & 0x7;
			remaining = 3;
			break;
		case 0b1110:
			code = x & 0xf;
			remaining = 2;
			break;
		case 0b1100:
		case 0b1101:
			code = x & 0x1f;
			remaining = 1;
			break;
		default:
			if (x & 0x80)
				throw std::runtime_error("unepected continuation byte");
			return x;
		}
		for (; remaining > 0; --remaining)
		{
			if (text.empty())
				return 0;
			const uchar y = uchar(text.front()); text.remove_prefix(1);
			if (y >> 6 != 2)
				throw std::runtime_error("continuation byte expected");
			code = (code << 6) | (y & 0x3f);
		}
		return code;
	}

	std::wstring wstr(std::string_view text)
	{
		std::wstring wide;
		while (!text.empty())
		{
			auto cp = popCodepoint(text);
			if (cp < 0x10000)
			{
				wide.push_back(static_cast<wchar_t>(cp));
				continue;
			}

			cp -= 0x10000;
			wide.push_back(static_cast<wchar_t>(0xd800 + (cp >> 10)));
			wide.push_back(static_cast<wchar_t>(0xdc00 + (cp & 0x03ff)));
		}
		return wide;
	}

	std::string utf8(int code)
	{
		std::string result;
		if (code < 0x80)
			result.push_back(static_cast<char>(code));
		else if (code < 0x800)
		{
			result.push_back(static_cast<char>(0xc0 | (code >> 6)));
			result.push_back(static_cast<char>(0x80 | (code & 0x3f)));
		}
		else if (code < 0x10000)
		{
			result.push_back(static_cast<char>(0xe0 | ((code >> 12) & 0x0f)));
			result.push_back(static_cast<char>(0x80 | ((code >>  6) & 0x3f)));
			result.push_back(static_cast<char>(0x80 | (code & 0x3f)));
		}
		else
		{
			result.push_back(static_cast<char>(0xf0 | ((code >> 18) & 0x07)));
			result.push_back(static_cast<char>(0x80 | ((code >> 12) & 0x3f)));
			result.push_back(static_cast<char>(0x80 | ((code >>  6) & 0x3f)));
			result.push_back(static_cast<char>(0x80 | (code & 0x3f)));
		}

		return result;
	}

	static constexpr int font_texture_width = 2048;


	class Font::Data
	{
		friend class Font;

		FT_Face _face;
		unsigned _tex;
		const int _size;
		int _asc; // Ascender in pixels
		int _dsc; // Descender in pixels
		int _next_offset = 0;
		struct GlyphInfo
		{
			float offset;
			float width;
			int advance;
			int left;
		};
		std::unordered_map<int, GlyphInfo> _glyphs;

		const GlyphInfo& _init_glyph(int ch, bool drawing);

		float _draw_glyph(int ch, Point p, float height);
	public:
		Data(const std::string& name, int size) : _size(size)
		{
			FT_New_Face(freetype.lib, name.c_str(), 0, &_face);

			//FT_Set_Char_Size(
			//	_face,    /* handle to face object           */
			//	0,       /* char_width in 1/64th of points  */
			//	size * 64,   /* char_height in 1/64th of points */
			//	300,     /* horizontal device resolution    */
			//	300);   /* vertical device resolution      */

			FT_Set_Pixel_Sizes(
				_face,    /* handle to face object */
				0,        /* pixel_width           */
				_size);   /* pixel_height          */

			_asc = _face->size->metrics.ascender >> 6;
			_dsc = _face->size->metrics.descender >> 6;

			glGenTextures(1, &_tex);
			glBindTexture(GL_TEXTURE_2D, _tex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA4, font_texture_width, _asc-_dsc, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		}
		~Data()
		{
			glDeleteTextures(1, &_tex);

		}

		float offset(std::string_view text, float height);

		void drawLine(const Point& start, std::string_view text, float height, const Color& color);
	};

	Font::Font(const std::string& name, int size) : _data(std::make_shared<Data>(name, size)) { }

	void Font::drawLine(const Point & start, std::string_view text, float height, const Color & color)
	{
		if (_data)
			_data->drawLine(start, text, height, color);
	}

	float Font::offset(std::string_view text, float height)
	{
		return !_data ? 0.0f : _data->offset(text, height);
	}

	float Font::height() const
	{
		return static_cast<float>(!_data ? 0 : _data->_size);
	}


	void Font::Data::drawLine(const Point& start, std::string_view text, float height, const Color& color)
	{
		if (height == 0)
			height = float(_size);
		Point head = start;

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, _tex);

		glColor4fv(&color.r);
		glBegin(GL_QUADS);
		while (int ch = popCodepoint(text))
			head.x = _draw_glyph(ch, head, height/_size);
		glEnd();

		glDisable(GL_TEXTURE_2D);
	}
	float Font::Data::offset(std::string_view text, float height)
	{
		const float factor = (height == 0 ? 1 : (height / _size));

		float result = 0;
		while (int ch = popCodepoint(text))
			result += _init_glyph(ch, false).advance*factor;
		return result;
	}
	float Font::Data::_draw_glyph(int ch, Point p, float factor)
	{
		auto& info = _init_glyph(ch, true);

		const auto s0 = info.offset;
		const auto s1 = info.width + s0;

		const float lift = (_asc - _dsc - _size)*factor;
		const float left = info.left*factor;

		const float width = info.width * font_texture_width * factor;
		const float x_1 = p.x + width;
		const float y_1 = p.y + _size*factor;

		glTexCoord2f(s0, 1); glVertex2f(p.x+left, y_1 - lift * float(_dsc) / (_asc - _dsc));
		glTexCoord2f(s1, 1); glVertex2f(x_1+left, y_1 - lift * float(_dsc) / (_asc - _dsc));
		glTexCoord2f(s1, 0); glVertex2f(x_1+left, p.y - lift * float(_asc) / (_asc - _dsc));
		glTexCoord2f(s0, 0); glVertex2f(p.x+left, p.y - lift * float(_asc) / (_asc - _dsc));

		return p.x + info.advance * factor;
	}

	const Font::Data::GlyphInfo& Font::Data::_init_glyph(int ch, bool drawing)
	{
		auto info = _glyphs.find(ch);
		if (info == _glyphs.end())
		{
			wchar_t str[3] = { wchar_t(ch), 0, 0 };
			if (ch >= 0x10000)
			{
				const int chm = ch - 0x10000;
				str[0] = wchar_t((chm >> 10) + 0xd800);
				str[1] = wchar_t((chm & ((1 << 10) - 1)) + 0xdc00);
			}
			FT_UInt  glyph_index = FT_Get_Char_Index(_face, ch);
			FT_Load_Glyph(_face, glyph_index, FT_LOAD_DEFAULT);
			FT_Render_Glyph(_face->glyph, FT_RENDER_MODE_NORMAL);

			const auto& bitmap = _face->glyph->bitmap;
			const int width = bitmap.width;

			if (drawing) glEnd();
			else         glBindTexture(GL_TEXTURE_2D, _tex);
			const auto yoff = _asc - _face->glyph->bitmap_top;
			glTexSubImage2D(GL_TEXTURE_2D, 0, _next_offset, yoff, 
				bitmap.pitch, 
				bitmap.rows, GL_ALPHA, GL_UNSIGNED_BYTE, bitmap.buffer);
			if (drawing) glBegin(GL_QUADS);
				

			GlyphInfo new_info;
			new_info.offset = float(_next_offset) / font_texture_width;
			new_info.width = float(width) / font_texture_width;
			new_info.advance = _face->glyph->advance.x>>6;
			new_info.left = _face->glyph->bitmap_left;

			_next_offset += width + 1;

			info = _glyphs.emplace(ch, new_info).first;
		}
		return info->second;
	}

}
