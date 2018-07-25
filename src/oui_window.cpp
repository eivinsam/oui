#include "..\include\oui_window.h"
/* An example of the minimal Win32 & OpenGL program.  It only works in
16 bit color modes or higher (since it doesn't create a
palette). */

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windowsx.h>			/* must include this before GL/gl.h */
#include <ShellScalingApi.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <unordered_map>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "Shcore.lib")

#include <oui_window.h>
#include <oui_unicode.h>
#include <oui_debug.h>

namespace oui
{
	Input input;

	HWND _focus_wnd = nullptr;

	std::unordered_map<HWND, Window*> windows;

	class SystemWindow;
	LONG WINAPI WindowProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		static std::unordered_map<UINT, std::string> msg_names =
		{
		{ WM_NULL	 , "WM_NULL"     },
		{ WM_CREATE	 , "WM_CREATE"   },
		{ WM_DESTROY , "WM_DESTROY"  },
		{ WM_MOVE	 , "WM_MOVE"     },
		{ WM_SIZE	 , "WM_SIZE"     },
		{ WM_ACTIVATE, "WM_ACTIVATE" },
		{ WM_PAINT, "WM_PAINT" },
		{ WM_CLOSE, "WM_CLOSE" },
		{ WM_QUIT, "WM_QUIT" },
		{ WM_ERASEBKGND, "WM_ERASEBKGND" },
		{ WM_COMMAND, "WM_COMMAND" },
		{ WM_SHOWWINDOW, "WM_SHOWWINDOW" },
		{ WM_ACTIVATEAPP, "WM_ACTIVATEAPP" },
		{ WM_GETMINMAXINFO, "WM_GETMINMAXINFO" },
		{ WM_WINDOWPOSCHANGING, "WM_WINDOWPOSCHANGING" },
		{ WM_WINDOWPOSCHANGED, "WM_WINDOWPOSCHANGED" },
		{ WM_CHAR, "WM_CHAR " },
		{ WM_KEYDOWN, "WM_KEYDOWN "},
		{ WM_KEYUP, "WM_KEYUP" },
		{ WM_ENTERSIZEMOVE, "WM_ENTERSIZEMOVE" },
		{ WM_EXITSIZEMOVE, "WM_EXITSIZEMOVE" },
		{ WM_MOUSEMOVE, "" },
		{ WM_LBUTTONDOWN, "WM_LBUTTONDOWN" },
		{ WM_LBUTTONUP, "WM_LBUTTONUP" },
		{ WM_RBUTTONDOWN, "WM_RBUTTONDOWN" },
		{ WM_RBUTTONUP, "WM_RBUTTONUP" },
		{ WM_SETFOCUS, "WM_SETFOCUS" },
		{ WM_KILLFOCUS, "WM_KILLFOCUS" },
		{ WM_NCCREATE,	"WM_NCCREATE" },
		{ WM_NCDESTROY, "WM_NCDESTROY" },
		{ WM_NCCALCSIZE, "WM_NCCALCSIZE" },
		{ WM_NCHITTEST, "" },
		{ WM_NCPAINT, "WM_NCPAINT" },
		{ WM_NCACTIVATE, "WM_NCACTIVATE" },
		{ WM_NCMOUSEMOVE, "" },
		{ WM_SETCURSOR, "" },
		{ WM_GETICON, "WM_GETICON" },
		{ 0x0090, "WM_UAHDESTROYWINDOW" },
		{ WM_SYSKEYDOWN, "WM_SYSKEYDOWN" },
		{ WM_SYSKEYUP  , "WM_SYSKEYUP" },
		{ WM_SYSCOMMAND, "WM_SYSCOMMAND" },
		{ WM_IME_SETCONTEXT, "WM_IME_SETCONTEXT" },
		{ WM_IME_NOTIFY, "WM_IME_NOTIFY" },
		{ WM_DWMNCRENDERINGCHANGED, "WM_DWMNCRENDERINGCHANGED" }
		};
		auto found = msg_names.find(msg);
		if (found == msg_names.end())
			debug::println(std::to_string(msg));
		else if (!found->second.empty())
			debug::println(found->second);

		auto get_point_lparam = [](LPARAM p) { return oui::Point{ float(GET_X_LPARAM(p)), float(GET_Y_LPARAM(p)) }; };
		auto get_vector_lparam = [](LPARAM p) { return oui::Vector{ float(GET_X_LPARAM(p)), float(GET_Y_LPARAM(p)) }; };

		const auto window = [wnd] 
		{ 
			auto found = windows.find(wnd); 
			return found == windows.end() ? nullptr : found->second; 
		}();

		static bool in_sizemove = false;
		static const auto apply_size = [](HWND wnd, Window& window)
		{
			debug::println(std::to_string(window.size.x) + "x" + std::to_string(window.size.y));
			glViewport(0, 0, GLsizei(window.size.x), GLsizei(window.size.y));

			if (window.resize)
				window.resize(window.size);

			RedrawWindow(wnd, nullptr, nullptr, RDW_INTERNALPAINT);
		};

		switch (msg)
		{
		case WM_WINDOWPOSCHANGED:
		{
			auto info = reinterpret_cast<WINDOWPOS*>(lParam);

			if (IsIconic(wnd))
				return 0;

			if ((info->flags & SWP_SHOWWINDOW) ||
				(info->flags & SWP_NOSIZE) == 0)
			{
				if (window)
				{
					RECT cr;
					GetClientRect(wnd, &cr);
					window->size = { float(cr.right - cr.left), float(cr.bottom - cr.top) };
					if (!in_sizemove)
						apply_size(wnd, *window);
				}
			}
			return 0; // DefWindowProc(wnd, msg, wParam, lParam);
		}
		case WM_ENTERSIZEMOVE:
			in_sizemove = true;
			return 0;
		case WM_EXITSIZEMOVE:
			in_sizemove = false;
			if (window)
				apply_size(wnd, *window);
			return 0;
		case WM_CHAR:
			if (input.character)
				input.character(wParam);
			return 0;
		case WM_DESTROY:
			if (window)
			{
				window->close();
				if (windows.size() == 1)
					PostQuitMessage(0);
			}
			return 0;
		case WM_SETFOCUS:
			_focus_wnd = wnd;
			RedrawWindow(wnd, nullptr, nullptr, RDW_INTERNALPAINT);
			return 0;
		case WM_KILLFOCUS:
			_focus_wnd = nullptr;
			RedrawWindow(wnd, nullptr, nullptr, RDW_INTERNALPAINT);
			return 0;
		case WM_MOUSEMOVE:
			input.mouse.move(get_point_lparam(lParam));
			return 0;
		case WM_LBUTTONDOWN:
			SetCapture(wnd);
			input.mouse.press(get_point_lparam(lParam));
			return 0;
		case WM_RBUTTONDOWN:
			SetCapture(wnd);
			input.mouse.longPress(get_point_lparam(lParam));
			return 0;
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
			ReleaseCapture();
			input.mouse.release(get_point_lparam(lParam));
			return 0;
		case WM_KEYDOWN:
			if (input.keydown)
				input.keydown(static_cast<Key>(wParam));
		default:
			return DefWindowProc(wnd, msg, wParam, lParam);
		}
	}

	bool dispatchMessages()
	{
		MSG   msg;				/* message */
		int status;
		while ((status = GetMessage(&msg, NULL, 0,0)) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_PAINT)
				return true;
		}
		if (status != 0)
			return false;
		if (msg.message != WM_QUIT)
			return true;
		debug::println("WM_QUIT");
		return false;
	}

	class SystemWindow
	{
		friend class Renderer;
		HWND _wnd;
		HDC _dc;
	public:
		SystemWindow(const Window::Description& desc, std::optional<int> custom_pixel_format = std::nullopt)
		{
			static HINSTANCE hInstance = 0;

			/* only register the window class once - use hInstance as a flag. */
			if (!hInstance) {
				hInstance = GetModuleHandle(NULL);
				WNDCLASS    wc;
				wc.style = CS_OWNDC;
				wc.lpfnWndProc = (WNDPROC)WindowProc;
				wc.cbClsExtra = 0;
				wc.cbWndExtra = 0;
				wc.hInstance = hInstance;
				wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
				wc.hCursor = LoadCursor(NULL, IDC_ARROW);
				wc.hbrBackground = NULL;
				wc.lpszMenuName = NULL;
				wc.lpszClassName = "OpenGL";

				if (!RegisterClass(&wc))
					throw std::runtime_error("RegisterClass() failed:  Cannot register window class");
			}

			_wnd = CreateWindow("OpenGL", desc.title.c_str(), WS_OVERLAPPEDWINDOW |
								WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
								CW_USEDEFAULT, CW_USEDEFAULT, desc.width, desc.height, NULL, NULL, hInstance, NULL);
			if (_wnd == NULL)
				throw std::runtime_error("CreateWindow() failed:  Cannot create a window.");

			_dc = GetDC(_wnd);

			PIXELFORMATDESCRIPTOR pfd;
			memset(&pfd, 0, sizeof(pfd));
			pfd.nSize = sizeof(pfd);
			pfd.nVersion = 1;
			pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
			pfd.iPixelType = PFD_TYPE_RGBA;
			pfd.cColorBits = 32;

			const int pf = custom_pixel_format.value_or(ChoosePixelFormat(_dc, &pfd));
			if (pf == 0)
				throw std::runtime_error("ChoosePixelFormat() failed:  Cannot find a suitable pixel format");

			if (SetPixelFormat(_dc, pf, &pfd) == FALSE)
				throw std::runtime_error("SetPixelFormat() failed:  Cannot set format specified");
		}
		SystemWindow(const SystemWindow&) = delete;
		SystemWindow(SystemWindow&& b) : _wnd(b._wnd), _dc(b._dc) { b._wnd = NULL; b._dc = NULL; }
		~SystemWindow()
		{
			if (_dc != NULL)
				ReleaseDC(_wnd, _dc);
			if (_wnd != NULL)
			{
				windows.erase(_wnd);
				DestroyWindow(_wnd);
				//PostMessage(NULL, WM_PAINT, 0, 0);
				//dispatchMessages();
			}
		}

		void registrate(Window& w) { windows[_wnd] = &w; }

		void show(int mode) const { ShowWindow(_wnd, mode); }

		void swapBuffers() const
		{
			SwapBuffers(_dc);
		}

		bool focus() const { return _focus_wnd == _wnd; }

		void requestRedraw()
		{
			RedrawWindow(_wnd, nullptr, nullptr, RDW_INTERNALPAINT);
		}

		void title(std::string_view text)
		{
			SetWindowTextW(_wnd, wstr(text).c_str());
		}
	};

	class Renderer
	{
		HGLRC _rc;

		static std::optional<int> maybe_fancy_format(const Window::Description& desc)
		{
			if (desc.sampleCount <= 1)
				return std::nullopt;

			int format = 0;
			auto dummy_desc = desc;
			dummy_desc.sampleCount = 1;
			Renderer dummy(dummy_desc);

			int int_attributes[] =
			{
				WGL_SUPPORT_OPENGL_ARB,GL_TRUE,
				WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
				WGL_COLOR_BITS_ARB,24,
				WGL_ALPHA_BITS_ARB,8,
				WGL_DEPTH_BITS_ARB,16,
				WGL_STENCIL_BITS_ARB,0,
				WGL_DOUBLE_BUFFER_ARB,GL_TRUE,
				WGL_SAMPLE_BUFFERS_ARB,GL_TRUE,
				WGL_SAMPLES_ARB, desc.sampleCount,
				0, 0
			};
			float float_attributes[] = { 0, 0 };

			UINT formatc = 0;
			wglChoosePixelFormatARB(dummy.window._dc, int_attributes, float_attributes, 1, &format, &formatc);
			if (formatc == 0)
				throw std::runtime_error("Failed to find appropriate pixel format");

			return format;
		}
	public:
		SystemWindow window;

		Renderer(const Window::Description& desc) : window(desc, maybe_fancy_format(desc))
		{
			static bool glew_inited = false;

			_rc = wglCreateContext(window._dc);
			wglMakeCurrent(window._dc, _rc);

			//if (!glew_inited)
				glew_inited = glewInit() == GLEW_OK;
			wglSwapIntervalEXT(1);
		}
		Renderer(Renderer&& b) : _rc(b._rc), window(std::move(b.window)) { b._rc = NULL; }
		Renderer(const Renderer&) = delete;
		~Renderer()
		{
			if (_rc != NULL)
			{
				wglMakeCurrent(NULL, NULL);
				wglDeleteContext(_rc);
			}
		}
	};
	Window::Window(const Description& desc)
	{
		SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
		_renderer = std::make_unique<Renderer>(desc);
		_renderer->window.registrate(*this);
		_renderer->window.show(true);
	}
	Window::~Window() { }

	bool Window::update()
	{
		_renderer->window.swapBuffers();
		glFlush();
		_open &= oui::dispatchMessages();
		input.mouse.takeAll();
		if (!_open)
			return false;

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0, size.x, size.y, 0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		return true;
	}
	void Window::clear(const Color & c)
	{
		glClearColor(c.r, c.g, c.b, 1);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	unsigned Window::dpi() const
	{
		const POINT home = { 0 ,0 };
		UINT xdpi, ydpi;
		GetDpiForMonitor(MonitorFromPoint(home, MONITOR_DEFAULTTOPRIMARY), MDT_DEFAULT, &xdpi, &ydpi);
		return unsigned(sqrt(xdpi*ydpi));
	}
	bool Window::focus() const
	{
		return _renderer->window.focus();
	}
	void Window::redraw()
	{
		_renderer->window.requestRedraw();
	}
	void Window::title(std::string_view text)
	{
		_renderer->window.title(text);
	}
}
