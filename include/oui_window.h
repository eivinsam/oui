#pragma once

#include <string>
#include <oui.h>

namespace oui
{
	class Renderer;

	class Window
	{
		std::unique_ptr<Renderer> _renderer;
		Rectangle _area;
		bool _open = true;
	public:

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

		bool open() const { return _open; }

		unsigned dpi() const;
		inline float dpiFactor() const { return dpi() / 96.0f; }

		const Rectangle& area() const { return _area; }
	};
}