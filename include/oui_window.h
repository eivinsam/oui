#pragma once

#include <string>
#include <oui.h>

namespace oui
{
	class Renderer;

	class Window
	{
		std::unique_ptr<Renderer> _renderer;
		volatile bool _open = true;
	public:
		Vector size;

		struct Description
		{
			std::string title;
			int width = 640;
			int height = 480;
			int	sampleCount = 1;
		};

		Window(const Description&);
		~Window();

		bool update();

		void clear(const Color&);

		void close();
		constexpr bool open() const { return _open; }

		unsigned dpi() const;
		inline float dpiFactor() const { return dpi() / 96.0f; }

		constexpr Rectangle area() const { return align::topLeft(origo).size(size); }

		bool focus() const;

		void redraw();

		void title(std::string_view text);

		Input::Handler<Vector> resize;
	};
}