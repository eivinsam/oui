#pragma once

#include <unordered_map>

#include <oui.h>
#include <oui_unicode.h>

namespace oui
{
	enum class NativeFont : char { sans, serif, mono };
	enum class FontFace : char { normal, bold, italic, bold_italic };

	std::string resolve(NativeFont, FontFace = FontFace::normal);

	class Font
	{
		class Data;
		std::shared_ptr<Data> _data;

	public:
		Font(const std::string& name, int size);

		void drawLine(const Point& start, std::string_view, float height = 0, const Color& = colors::white);
		void drawLine(const Point& start, std::string_view text, const Color& color, float height = 0) 
		{
			return drawLine(start, text, height, color);
		}

		float offset(std::string_view text, float height = 0);

		float height() const;
	};

	class VectorFont
	{
		class Data;
		std::shared_ptr<Data> _data;
	public:
		VectorFont(const std::string& name);

		float drawLine(const Point& start, std::string_view, float height, const Color& = colors::white);
		float drawLine(const Point& start, std::string_view text, const Color& color, float height)
		{
			return drawLine(start, text, height, color);
		}

		float offset(std::string_view text, float height);
	};
}