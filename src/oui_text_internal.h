#pragma once

#include <oui_text.h>

#include <GL/glew.h>

#include <ft2build.h>
#include FT_FREETYPE_H

inline struct FreeTypeInit
{
	FT_Library lib;

	FreeTypeInit()
	{
		FT_Init_FreeType(&lib);
	}
	~FreeTypeInit()
	{

	}
} freetype;

