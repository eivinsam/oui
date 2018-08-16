#include <oui_debug.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace oui::debug
{
	void print(const char* text)
	{
		OutputDebugStringA(text);
	}
}
