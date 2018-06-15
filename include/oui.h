#pragma once

#include <optional>
#include <chrono>
#include <functional>

namespace oui
{
	template <class A, class B>
	constexpr auto min(A a, B b) { return a < b ? a : b; }
	template <class A, class B>
	constexpr auto max(A a, B b) { return a < b ? b : a; }

	struct Color
	{
		float r;
		float g;
		float b;
		float a = 1;
	};

	inline constexpr Color operator+(const Color& a, const Color& b) { return { a.r + b.r, a.g + b.g, a.b + b.b, a.a + b.a }; }

	inline constexpr Color operator*(const Color& c, double s) { const auto fs = static_cast<float>(s); return { c.r*fs, c.g*fs, c.b*fs, c.a*fs }; }
	inline constexpr Color operator*(double s, const Color& c) { const auto fs = static_cast<float>(s); return { c.r*fs, c.g*fs, c.b*fs, c.a*fs }; }
	inline constexpr Color operator/(const Color& c, double s) { const auto fs = static_cast<float>(s); return { c.r/fs, c.g/fs, c.b/fs, c.a/fs }; }

	namespace colors
	{
		static constexpr Color black{ 0, 0, 0 };
		static constexpr Color white{ 1, 1, 1 };

		static constexpr Color red{ 1, 0, 0 };
		static constexpr Color green{ 0, 1, 0 };
		static constexpr Color blue{ 0, 0, 1 };

		static constexpr Color yellow{ 1, 1, 0 };
		static constexpr Color cyan{ 0, 1, 1 };
		static constexpr Color magenta{ 1, 0, 1 };
	}


	struct Ratio
	{
		float value;

		Ratio() = default;
		explicit constexpr Ratio(float v) : value(v) { }

		friend constexpr float operator*(float x, Ratio f) { return x*f.value; }
		friend constexpr float operator*(Ratio f, float x) { return f.value*x; }
	};

	struct Vector
	{
		float x = 0;
		float y = 0;
	};

	inline constexpr Vector operator+(Vector a, Vector b) { return { a.x + b.x, a.y + b.y }; }
	inline constexpr Vector operator-(Vector a, Vector b) { return { a.x - b.x, a.y - b.y }; }
	inline constexpr Vector operator*(Vector a, Vector b) { return { a.x * b.x, a.y * b.y }; }
	inline constexpr Vector operator/(Vector a, Vector b) { return { a.x / b.x, a.y / b.y }; }

	inline constexpr Vector operator*(Vector v, float c) { return { v.x*c, v.y*c }; }
	inline constexpr Vector operator*(float c, Vector v) { return { v.x*c, v.y*c }; }
	inline constexpr Vector operator/(Vector v, float c) { return { v.x/c, v.y/c }; }

	inline constexpr float   dot(Vector a, Vector b) { return a.x*b.x + a.y*b.y; }
	inline constexpr float cross(Vector a, Vector b) { return a.x*b.y - a.y*b.x; }

	inline float angle(Vector a, Vector b) noexcept { return acos(dot(a, b) / sqrt(dot(a, a)*dot(b, b))); }

	static constexpr struct {} origo;

	struct Rectangle;
	struct Point
	{
		float x = 0;
		float y = 0;

		constexpr Point() = default;
		constexpr Point(decltype(origo)) { }
		constexpr Point(float x, float y) : x(x), y(y) { }

		constexpr bool in(const Rectangle&) const;
	};

	inline constexpr Point operator+(Vector v, decltype(origo)) { return { v.x, v.y }; }
	inline constexpr Point operator+(decltype(origo), Vector v) { return { v.x, v.y }; }

	inline constexpr Vector operator-(Point p, decltype(origo)) { return { +p.x, +p.y }; }
	inline constexpr Vector operator-(decltype(origo), Point p) { return { -p.x, -p.y }; }

	inline constexpr Point operator+(Point p, Vector v) { return { p.x + v.x, p.y + v.y }; }
	inline constexpr Point operator-(Point p, Vector v) { return { p.x - v.x, p.y - v.y }; }
	inline constexpr Point operator+(Vector v, Point p) { return { v.x + p.x, v.y + p.y }; }
	inline constexpr Vector operator-(Point a, Point b) { return { a.x - b.x, a.y - b.y }; }
	
	struct Rectangle
	{
		Point min; // Top left corner
		Point max; // Bottom right corner

		constexpr float height() const { return max.y - min.y; }
		constexpr float width() const { return max.x - min.x; }

		constexpr Vector size() const { return max - min; }

		constexpr Point center() const { return min + (max - min) / 2; }

		constexpr Rectangle popLeft(int   dx) { return popLeft(static_cast<float>(dx)); }
		constexpr Rectangle popLeft(float dx)
		{
			dx = oui::min(dx, width());
			const float oldx = min.x;
			min.x += dx;
			return { { oldx, min.y }, { min.x, max.y } };
		}
		constexpr Rectangle popRight(int   dx) { return popRight(static_cast<float>(dx)); }
		constexpr Rectangle popRight(float dx)
		{
			dx = oui::min(dx, width());
			const float oldx = max.x;
			max.x -= dx;
			return { { max.x, min.y }, { oldx, max.y } };
		}

		constexpr Rectangle popTop(int   dx) { return popTop(static_cast<float>(dx)); }
		constexpr Rectangle popTop(float dy)
		{ 
			dy = oui::min(dy, height());
			const float oldy = min.y;
			min.y += dy; 
			return { { min.x, oldy }, { max.x, min.y } }; 
		}
		constexpr Rectangle popBottom(int   dx) { return popBottom(static_cast<float>(dx)); }
		constexpr Rectangle popBottom(float dy)
		{
			dy = oui::min(dy, height());
			const float oldy = max.y;
			max.y -= dy;
			return { { min.x, max.y }, { max.x, oldy } };
		}

		constexpr Rectangle popLeft(Ratio f)   { return popLeft(width()*f); }
		constexpr Rectangle popRight(Ratio f)  { return popRight(width()*f); }
		constexpr Rectangle popTop(Ratio f)    { return popTop(height()*f); }
		constexpr Rectangle popBottom(Ratio f) { return popBottom(height()*f); }

		constexpr Rectangle shrink(float trim) const
		{
			Rectangle r = *this;
			r.min.x += trim;
			r.min.y += trim;
			r.max.x -= trim;
			r.max.y -= trim;
			if (r.min.x > r.max.x)
				r.min.x = r.max.x = (r.min.x + r.max.x) / 2;
			if (r.min.y > r.max.y)
				r.min.y = r.max.y = (r.min.y + r.max.y) / 2;
			return *this;
		}
		constexpr Rectangle shrink(Ratio f) const
		{
			f.value = oui::max(0, oui::min(f.value, 1));
			const float dx = width()*f;
			const float dy = height()*f;
			return { { min.x + dx, min.y + dy }, { max.x - dx, max.y - dy } };
		}
	};

	inline constexpr Rectangle operator+(const Rectangle& r, Vector v) { return { r.min + v, r.max + v }; }
	inline constexpr Rectangle operator+(Vector v, const Rectangle& r) { return { r.min + v, r.max + v }; }

	void fill(const Rectangle&, const Color&);

	inline constexpr bool Point::in(const Rectangle& area) const
	{
		return
			area.min.x <= x && x < area.max.x &&
			area.min.y <= y && y < area.max.y;
	}

	struct FromScalar;
	struct Align
	{
		float c;

		constexpr float min(float x, float w) const { return x - c * w;	}
		constexpr float max(float x, float w) const { return x + (1 - c)*w; }

	};



	struct FromPoint;
	struct Align2
	{
		Vector c;

		constexpr Align2() = default;
		constexpr Align2(Vector c) : c{ c } { }
		constexpr Align2(float cx, float cy) : c{cx, cy} { }
		constexpr Align2(Align a) : c{ a.c, a.c } { }

		constexpr Point min(Point point, Vector size) const { return point - c * size; }
		constexpr Point max(Point point, Vector size) const { return point + (Vector{1, 1} - c)*size; }

		constexpr FromPoint operator()(Point) const;
		constexpr FromPoint operator()(Rectangle) const;
	};
	struct FromPoint
	{
		Point point;
		Align2 align;

		constexpr Rectangle size(Vector size) const
		{
			return { align.min(point, size), align.max(point, size) };
		}
	};
	inline constexpr FromPoint Align2::operator()(Point p) const { return { p, *this }; }
	inline constexpr FromPoint Align2::operator()(Rectangle r) const
	{
		return { r.min + c * (r.max - r.min), *this };
	}

	namespace align
	{
		static constexpr Align min{ 0.0f };
		static constexpr Align max{ 1.0f };
		static constexpr Align center{ 0.5f };

		static constexpr Align2 topLeft{ 0.0f, 0.0f };
		static constexpr Align2 topCenter{ 0.5f, 0.0f };
		static constexpr Align2 topRight{ 1.0f, 0.0f };
		static constexpr Align2 centerLeft{ 0.0f, 0.5f };
		static constexpr Align2 centerRight{ 1.0f, 0.5f };
		static constexpr Align2 bottomLeft{ 0.0f, 1.0f };
		static constexpr Align2 bottomCenter{ 0.5f, 1.0f };
		static constexpr Align2 bottomRight{ 1.0f, 1.0f };
	}

	template <class T>
	using Optional = std::optional<T>;
	using Time = std::chrono::time_point<std::chrono::high_resolution_clock>;

	inline Time now() noexcept { return std::chrono::high_resolution_clock::now(); }
	
	inline float duration(Time t0, Time t1) noexcept { return std::chrono::duration<float>(t1 - t0).count(); }

	template <class T>
	Optional<T> pop(Optional<T>& op) noexcept
	{
		Optional<T> result;
		std::swap(op, result);
		return result;
	}

	class Pointer
	{
		struct Context
		{
			Time time;
			Point position;
		};
		struct Down : Context
		{
			constexpr Down(Time time, Point point) : Context{ time, point } { }
			Optional<Context> up;
		};
		Optional<Context> _current;
		Optional<Vector> _delta;
		Optional<Down> _button;
	public:
		void move(const Point& new_position)
		{
			if (_current)
				_delta = _delta.value_or(Vector{ 0, 0 }) + (new_position - _current->position);
			_current = { now(), new_position };
		}
		void press(const Point& position)
		{
			_button = { now(), position };
		}
		void longPress(const Point& position)
		{
			_button = { now() - std::chrono::seconds(1), position };
		}
		void release(const Point& position)
		{
			if (_button)
				_button->up = { now(), position };
		}

		// Reset all complete states
		void takeAll() noexcept
		{
			_delta = std::nullopt;
			if (_button && _button->up)
				_button = std::nullopt;
		}


		bool hovering(const Rectangle& area) const noexcept { return _current && _current->position.in(area); }
		Optional<Point> holding(const Rectangle& area) const noexcept
		{
			if (_button && _button->position.in(area))
				return _current->position;
			return std::nullopt;
		}
		Optional<Vector> dragging(const Rectangle& area) noexcept
		{
			if (holding(area))
				return pop(_delta);
			return std::nullopt;
		}

		Optional<Point> pressed(const Rectangle& area) noexcept
		{
			if (holding(area) && 
				_button->up && _button->up->position.in(area) &&
				duration(_button->time, _button->up->time) < 1)
			{
				Point result = _button->up->position;
				_button.reset();
				return result;
			}
			return std::nullopt;
		}
		bool longPressed(const Rectangle& area) noexcept
		{
			if (holding(area) && 
				_button->up && _button->up->position.in(area) &&
				duration(_button->time, _button->up->time) >= 1)
			{
				_button.reset();
				return true;
			}
			return false;
		}
	};

	enum class Key
	{
		lButton = 1, rButton, cancel, mButton,
		backspace = 0x8, tab,
		enter = 0xd,
		space = 0x20, pgup, pgdn, end, home, left, up, right, down,
		ins = 0x2d, del
	};

	class Input
	{
	public:
		template <class... Args>
		using Handler = std::function<void(Args...)>;

		Pointer mouse;

		Handler<Key> keydown;
		Handler<int> character;
	};

	extern Input input;
}