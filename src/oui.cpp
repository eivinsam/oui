#include "..\include\oui.h"
#include "oui.h"

#include <GL/glew.h>

namespace oui
{
	void fill(const Rectangle& area, const Color& color)
	{
		glColor4f(color.r, color.g, color.b, color.a);
		glBegin(GL_QUADS);
		glVertex2f(area.min.x, area.min.y);
		glVertex2f(area.min.x, area.max.y);
		glVertex2f(area.max.x, area.max.y);
		glVertex2f(area.max.x, area.min.y);
		glEnd();
	}
	void line(const Point& a, const Point& b, const Color& color, float thickness)
	{
		glColor4f(color.r, color.g, color.b, color.a);
		glLineWidth(thickness);
		glBegin(GL_LINES);
		glVertex2f(a.x, a.y);
		glVertex2f(b.x, b.y);
		glEnd();
	}
	void shift(Vector offset)
	{
		glTranslatef(offset.x, offset.y, 0);
	}
}
