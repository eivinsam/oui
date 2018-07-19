#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include <oui.h>

namespace oui
{
	int popCodepoint(std::string_view& text);
	std::string utf8(int code);
	inline constexpr int utf8len(unsigned char ch)
	{
		switch (ch >> 4)
		{
		case 0xf: return 4;
		case 0xe: return 3;
		case 0xd: case 0xc: return 2;
		default:
			return (ch >> 7) ^ 1;
		}
	}

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

		void drawLine(const Point& start, std::string_view, float height, const Color& = colors::white);
		void drawLine(const Point& start, std::string_view text, const Color& color, float height)
		{
			return drawLine(start, text, height, color);
		}

		float offset(std::string_view text, float height);
	};
}