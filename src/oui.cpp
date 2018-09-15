#include "..\include\oui.h"
#include "oui.h"

#include <GL/glew.h>
#include <cassert>

namespace oui
{
	void set(const Color& color)
	{
		glColor4fv(&color.r);
	}
	void set(Blend blend)
	{
		switch (blend)
		{
		case Blend::normal:   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); return;
		case Blend::multiply: glBlendFunc(GL_DST_COLOR, GL_ZERO); return;
		default:
			assert(false);
		}
	}
	void set(LineThickness thickness)
	{
		glLineWidth(thickness.value);
	}

	void fill(const Rectangle& area)
	{
		glBegin(GL_QUADS);
		glVertex2f(area.min.x, area.min.y);
		glVertex2f(area.min.x, area.max.y);
		glVertex2f(area.max.x, area.max.y);
		glVertex2f(area.max.x, area.min.y);
		glEnd();
	}
	void line(const Point& a, const Point& b)
	{
		glBegin(GL_LINES);
		glVertex2fv(&a.x);
		glVertex2fv(&b.x);
		glEnd();
	}
	void shift(Vector offset)
	{
		glTranslatef(offset.x, offset.y, 0);
	}
}
