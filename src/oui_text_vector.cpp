#include "oui_text_internal.h"
#include FT_OUTLINE_H

extern "C"
{
#include "gpc/gpc.h"
}

#include <array>


template <class IT>
class Span
{
	IT _first;
	IT _last;
public:
	using value_type = typename std::iterator_traits<IT>::value_type;

	Span(IT first, IT last) : _first(first), _last(last) { }
	template <class S, class = std::enable_if_t<std::is_integral_v<S>>>
	Span(IT first, S size) : _first(first), _last(first + size) { }

	bool empty() const { return _first == _last; }
	auto size() const { return _last - _first; }

	auto& operator[](size_t i) { return *(_first + i); }
	auto& operator[](size_t i) const { return *(_first + i); }

	IT begin() const { return _first; }
	IT end() const { return _last; }
};

struct ToGPC
{
	float scale;

	constexpr gpc_vertex operator()(const FT_Vector& p) const noexcept { return { scale*p.x, scale*p.y }; }
};

struct Tesselator
{
	using VecArg = const FT_Vector*;
	static auto self(void* obj) { return static_cast<Tesselator*>(obj); }

	gpc_polygon poly;
	std::vector<gpc_vertex> partial_contour;
	ToGPC to_gpc;

	static FT_Outline_Funcs ofuncs;

	void end_contour()
	{
		if (partial_contour.empty())
			return;
		gpc_vertex_list contour
		{
			static_cast<int>(partial_contour.size()),
			partial_contour.data()
		};
		gpc_add_contour(&poly, &contour, 0);
		partial_contour.clear();
	}

	int begin_contour(VecArg from)
	{
		end_contour();
		partial_contour.push_back(to_gpc(*from));
		return 0;
	}

	int add_linear(VecArg to)
	{
		partial_contour.push_back(to_gpc(*to));
		return 0;
	}

	int add_quad(VecArg c, VecArg to)
	{
		struct Controls
		{
			gpc_vertex p0, p1, p2;
		};
		struct Coef
		{
			std::array<float, 3> coef;

			constexpr Coef(float t) : Coef(t, 1 - t) {}

			gpc_vertex apply(const Controls& ctrl) const
			{
				return
				{
					coef[0] * ctrl.p0.x + coef[1] * ctrl.p1.x + coef[2] * ctrl.p2.x,
					coef[0] * ctrl.p0.y + coef[1] * ctrl.p1.y + coef[2] * ctrl.p2.y
				};
			}
		private:
			constexpr Coef(float t, float s) : coef{ s*s, 2 * s*t, t*t } {}

		};
		static constexpr Coef coefs[] = { 0.25f, 0.5f, 0.75f };

		const Controls ctrl{ partial_contour.back(), to_gpc(*c), to_gpc(*to) };

		for (auto&& c : coefs)
			partial_contour.push_back(c.apply(ctrl));
		partial_contour.push_back(ctrl.p2);
		return 0;
	}

	int add_cube(VecArg ca, VecArg cb, VecArg to)
	{
		struct Controls
		{
			gpc_vertex p0, p1, p2, p3;
		};
		struct Coef
		{
			std::array<float, 4> coef;

			constexpr Coef(float t) : Coef(t, 1 - t) { }

			gpc_vertex apply(const Controls& ctrl) const
			{
				return
				{
					coef[0] * ctrl.p0.x + coef[1] * ctrl.p1.x + coef[2] * ctrl.p2.x + coef[3] * ctrl.p3.x,
					coef[0] * ctrl.p0.y + coef[1] * ctrl.p1.y + coef[2] * ctrl.p2.y + coef[3] * ctrl.p3.y
				};
			}
		private:
			constexpr Coef(float t, float s) : coef{ s*s*s, 3 * s*s*t, 3 * s*t*t, t*t*t } {}
		};
		static constexpr Coef coefs[] = { 0.25f, 0.5f, 0.75f };

		const Controls ctrl{ partial_contour.back(), to_gpc(*ca), to_gpc(*cb), to_gpc(*to) };

		for (auto&& c : coefs)
			partial_contour.push_back(c.apply(ctrl));
		partial_contour.push_back(ctrl.p3);
		return 0;
	}

	gpc_tristrip generate_tristrip()
	{
		end_contour();

		gpc_tristrip result{ 0, nullptr };
		gpc_polygon_to_tristrip(&poly, &result);
		return result;
	}

	Tesselator(ToGPC to_gpc) : poly{ 0, nullptr, nullptr }, to_gpc(to_gpc)
	{
		ofuncs.shift = 0;
		ofuncs.delta = 0;

		ofuncs.move_to = [](VecArg to, void* tess) { return self(tess)->begin_contour(to); };
		ofuncs.line_to = [](VecArg to, void* tess) { return self(tess)->add_linear(to); };
		ofuncs.conic_to = [](VecArg c, VecArg to, void* tess) { return self(tess)->add_quad(c, to); };
		ofuncs.cubic_to = [](VecArg ca, VecArg cb, VecArg to, void* tess)
		{ return self(tess)->add_cube(ca, cb, to); };
	}
	~Tesselator()
	{
		gpc_free_polygon(&poly);
	}

};
FT_Outline_Funcs Tesselator::ofuncs;

namespace oui
{
	class VectorFont::Data
	{
		friend class VectorFont;

		FT_Face _face;
		float _rem;
		GLuint vao;

		struct GlyphInfo
		{
			float advance;
			GLuint vbo;
			GLuint vbo_size;
		};
		std::unordered_map<int, GlyphInfo> _glyphs;

		const GlyphInfo& _init_glyph(int ch, bool drawing);

		float _draw_glyph(int ch, Point p, float height);
	public:
		Data(const std::string& name) 
		{
			glGenVertexArrays(1, &vao);

			FT_New_Face(freetype.lib, name.c_str(), 0, &_face);

			_rem = 1.0f / _face->units_per_EM;
		}

		float offset(std::string_view text, float height);

		float drawLine(const Point& start, std::string_view text, float height, const Color& color);
	};


	const VectorFont::Data::GlyphInfo& VectorFont::Data::_init_glyph(int ch, bool drawing)
	{

		auto info = _glyphs.find(ch);
		if (info == _glyphs.end())
		{
			FT_UInt  glyph_index = FT_Get_Char_Index(_face, ch);
			FT_Load_Glyph(_face, glyph_index, FT_LOAD_NO_SCALE);

			{
				GlyphInfo new_info;
				new_info.advance = _rem*_face->glyph->advance.x;

				info = _glyphs.emplace(ch, new_info).first;
			}
			const auto& outline = _face->glyph->outline;
			auto& new_info = info->second;

			Tesselator tess({ _rem });
			FT_Outline_Decompose(&_face->glyph->outline, &Tesselator::ofuncs, &tess);
			auto strips = tess.generate_tristrip();

			static const auto to_point = [](auto&& p) -> Point
			{ return { static_cast<float>(p.x), static_cast<float>(p.y) }; };

			std::vector<Point> strip_data;
			for (const auto& strip : Span{ strips.strip, strips.num_strips })
			{
				strip_data.push_back(to_point(strip.vertex[0]));
				for (const auto& vertex : Span{ strip.vertex, strip.num_vertices })
					strip_data.push_back(to_point(vertex));
				strip_data.push_back(to_point(strip.vertex[strip.num_vertices - 1]));
			}

			glGenBuffers(1, &new_info.vbo);
			new_info.vbo_size = strip_data.size();
			glBindBuffer(GL_ARRAY_BUFFER, new_info.vbo);
			glBufferData(GL_ARRAY_BUFFER, new_info.vbo_size * sizeof(Point), strip_data.data(), GL_STATIC_DRAW);

			gpc_free_tristrip(&strips);
		}
		return info->second;
	}
	float VectorFont::Data::_draw_glyph(int ch, Point p, float factor)
	{
		auto& info = _init_glyph(ch, true);

		glBindBuffer(GL_ARRAY_BUFFER, info.vbo);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, info.vbo_size);

		glTranslatef(info.advance, 0, 0);

		return p.x + info.advance * factor;
	}

	float VectorFont::Data::offset(std::string_view text, float height)
	{
		float result = 0;
		while (int ch = popCodepoint(text))
			result += _init_glyph(ch, false).advance*height;
		return result;
	}
	float VectorFont::Data::drawLine(const Point& start, std::string_view text, float height, const Color& color)
	{
		Point head = start;

		glBindVertexArray(vao);
		glEnableVertexAttribArray(0);

		const auto asc = _face->ascender * _rem;
		const auto h = (_face->ascender - _face->descender) * _rem;

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(start.x, start.y, 0);
		glScalef(height, -height, height);
		glTranslatef(0, -asc+(h-1), 0);

		glColor4fv(&color.r);
		while (int ch = popCodepoint(text))
			head.x = _draw_glyph(ch, head, height);

		glPopMatrix();

		return head.x;
	}


	VectorFont::VectorFont(const std::string & name) :
		_data(std::make_shared<Data>(name)) { }

	float VectorFont::drawLine(const Point & start, std::string_view text, float height, const Color & color)
	{
		if (_data)
			return _data->drawLine(start, text, height, color);
		return 0;
	}

	float VectorFont::offset(std::string_view text, float height)
	{
		return !_data ? 0.0f : _data->offset(text, height);
	}
}
