#pragma once

#include <string>
#include <string_view>

namespace oui
{
	int popCodepoint(std::string_view& text);
	std::wstring wstr(std::string_view text);

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
}
