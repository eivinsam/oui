#include "..\include\oui_window.h"

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

static constexpr const char* msg_lookup[] =
{
	/*000*/"NULL", "CREATE", "DESTROY", "MOVE",
	/*004*/0, "SIZE", "ACTIVATE", "SETFOCUS",
	/*008*/"KILLFOCUS", 0, "ENABLE", "SETREDRAW",
	/*00c*/"SETTEXT", "GETTEXT", "GETTEXTLENGTH", "PAINT",
	/*010*/"CLOSE", "QUERYENDSESSION", "QUIT", "QUERYOPEN",
	/*014*/"ERASEBKGND", "SYSCOLORCHANGE", "ENDSESSION", 0,
	/*018*/"SHOWWINDOW", 0, "WININICHANGE", "DEVMODECHANGE",
	/*01c*/"ACTIVATEAPP", "FONTCHANGE", "TIMECHANGE", "CANCELMODE",
	/*020*/"SETCURSOR", "MOUSEACTIVATE", "CHILDACTIVATE", "QUEUESYNC",
	/*024*/"GETMINMAXINFO", 0, "WM_PAINTICON", "ICONERASEBKGND",
	/*028*/"NEXTDLGCTL", 0, "SPOOLERSTATUS", "DRAWITEM",
	/*02c*/"MEASUREITEM", "DELETEITEM", "VKEYTOITEM", "CHARTOITEM",
	/*030*/"SETFONT", "GETFONT", "SETHOTKEY", "GETHOTKEY",
	/*034*/0, 0, 0, "QUERYDRAGICON",
	/*038*/0, "COMPAREITEM", 0, 0,
	/*03c*/0, "GETOBJECT", 0, 0,
	/*040*/0, "COMPACTING", 0, 0,
	/*044*/"COMMNOTIFY", 0, "WINDOWPOSCHANGING", "WINDOWPOSCHANGED",
	/*048*/"POWER", 0, "COPYDATA", "CANCELJOURNAL",
	/*04c*/0, 0, "NOTIFY", 0,
	/*050*/"INPUTLANGCHANGEREQUEST", "INPUTLANGCHANGE", "TCARD", "HELP",
	/*054*/"USERCHANGED", "NOTIFYFORMAT", 0,0,
	/*058*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*068*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*078*/0,0,0,"CONTEXTMENU", 
	/*07c*/"STYLECHANGING", "STYLECHANGED", "DISPLAYCHANGE", "GETICON",
	/*080*/"SETICON", "NCCREATE", "NCDESTROY", "NCCALCSIZE", 
	/*084*/"NCHITTEST", "NCPAINT", "NCACTIVATE", "GETDLGCODE",
	/*088*/"SYNCPAINT", 0, 0, 0, 0,0,0,0,
	/*090*/"UAHDESTROYWINDOW",0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*0a0*/"NCMOUSEMOVE", "NCLBUTTONDOWN", "NCLBUTTONUP", "NCLBUTTONDBLCLK", 
	/*0a4*/"NCRBUTTONDOWN", "NCRBUTTONUP", "NCRBUTTONDBLCLK", "NCMBUTTONDOWN",
	/*0a8*/"NCMBUTTONUP", "NCMBUTTONDBLCLK", 0, "NCXBUTTONDOWN",
	/*0ac*/"NCXBUTTONUP", "NCXBUTTONDBLCLK", 0, 0,
	/*0b0*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0, 
	/*0c0*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*0d0*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*0e0*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*0f0*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0, "INPUT_DEVICE_CHANGE", "WM_INPUT",
	/*100*/   "KEYDOWN",    "KEYUP",    "CHAR",    "DEADCHAR",
	/*104*/"SYSKEYDOWN", "SYSKEYUP", "SYSCHAR", "SYSDEADCHAR",
	/*108*/0, "UNICHAR", 0, 0, 
	/*10c*/0, "IME_STARTCOMPOSITION", "IME_ENDCOMPOSITION", "IME_COMPOSITION",
	/*110*/"INITDIALOG", "COMMAND", "SYSCOMMAND", "TIMER",
	/*114*/"HSCROLL", "VSCROLL", "INITMENU", "INITMENUPOPUP",
	/*118*/0, "GESTURE", "GESTURENOTIFY", 0,
	/*11c*/0,0,0,"MENUSELECT",
	/*120*/"MENUCHAR", "ENTERIDLE", "MENURBUTTONUP", "MENUDRAG",
	/*124*/"MENUGETOBJECT", "UNINITMENUPOPUP", "MENUCOMMAND", "CHANGEUISTATE",
	/*128*/"UPDATEUISTATE", "QUERYUISTATE", 0, 0, 0,0,0,0,
	/*130*/0,0, "CTLCOLORMSGBOX", "CTLCOLOREDIT", 
	/*134*/"CTLCOLORLISTBOX", "CTLCOLORBTN", "CTLCOLORDLG", "CTLCOLORSCROLLBAR",
	/*138*/"CTLCOLORSTATIC",0,0,0, 0,0,0,0,
	/*140*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*150*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*160*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*170*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*180*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*190*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*1a0*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*1b0*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*1c0*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*1d0*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*1e0*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*1f0*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*200*/"MOUSEMOVE", "LBUTTONDOWN", "LBUTTONUP", "LBUTTONDBLCLK",
	/*204*/"RBUTTONDOWN", "RBUTTONUP", "RBUTTONDBLCLK", "MBUTTONDOWN",
	/*208*/"MBUTTONUP", "MBUTTONDBLCLK", "MOUSEWHEEL", "XBUTTONDOWN",
	/*20c*/"XBUTTONUP", "XBUTTONDBLCLK", "MOUSEWHEEL", 0, 
	/*210*/"PARENTNOTIFY", "ENTERMENULOOP", "EXITMENULOOP", "NEXTMENU",
	/*214*/"SIZING", "CAPTURECHANGED", "MOVING", 0,
	/*218*/"POWERBROADCAST", "DEVICECHANGE", 0,0, 0,0,0,0,
	/*220*/"MDICREATE", "MDIDESTROY", "MDIACTIVATE", "MDIRESTORE", 
	/*224*/"MDINEXT", "MDIMAXIMIZE", "MDITILE", "MDICASCADE",
	/*228*/"MDIICONARRANGE", "MDIGETACTIVE", 0,0, 0,0,0,0,
	/*230*/"MDISETMENU", "ENTERSIZEMOVE", "EXITSIZEMOVE", "DROPFILES",
	/*234*/"MDIREFRESHMENU", 0,0,0,
	/*238*/"POINTERDEVICECHANGE", "POINTERDEVICEINRANGE", "POINTERDEVICEOUTOFRANGE", 0, 0,0,0,0,
	/*240*/"TOUCH", "NCPOINTERUPDATE", "NCPOINTERDOWN", "NCPOINTERUP",
	/*244*/0,         "POINTERUPDATE",   "POINTERDOWN",   "POINTERUP",
	/*248*/0, "POINTERENTER", "POINTERLEAVE", "POINTERACTIVATE",
	/*24c*/"POINTERCAPTURECHANGED", "TOUCHHITTESTING", "POINTERWHEEL", "POINTERHWHEEL",
	/*250*/"POINTERHITTEST", "POINTERROUTEDTO", "POINTERROUTEDAWAY", "POINTERROUTEDRELEASED",
	/*254*/         0,0,0,0,  0,0,0,0, 0,0,0,0, 
	/*260*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*270*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*280*/0, "IME_SETCONTEXT", "IME_NOTIFY", "IME_CONTROL",
	/*284*/"IME_COMPOSITIONFULL", "IME_SELECT", "IME_CHAR", 0,
	/*288*/"IME_REQUEST", 0,0,0, 0,0,0,0,
	/*290*/"IME_KEYDOWN", "IME_KEYUP",0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*2a0*/"NCMOUSEHOVER", "MOUSEHOVER", "NCMOUSELEAVE", "MOUSELEAVE",
	/*2a4*/         0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*2b0*/0,"WTSSESSION_CHANGE",0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*2c0*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*2d0*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*2e0*/"DPICHANGED",0,"DPICHANGED_BEFOREPARENT","DPICHANGED_AFTERPARENT",
	/*2e4*/"GETDPISCALEDSIZE",0,0,0,  0,0,0,0, 0,0,0,0,
	/*2f0*/0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
	/*300*/"CUT", "COPY", "PASTE", "CLEAR",
	/*304*/"UNDO", "RENDERFORMAT", "RENDERALLFORMATS", "DESTROYCLIPBOARD",
	/*308*/"DRAWCLIPBOARD", "PAINTCLIPBOARD", "VSCROLLCLIPBOARD", "SIZECLIPBOARD",
	/*30c*/"ASKCBFORMATNAME", "CHANGECBCHAIN", "HSCROLLCLIPBOARD", "QUERYNEWPALETTE",
	/*310*/"PALETTEISCHANGING", "PALETTECHANGED", "HOTKEY", 0,
	/*314*/0, 0, 0, "PRINT",
	/*318*/"PRINTCLIENT", "APPCOMMAND", "THEMECHANGED", 0,
	/*31c*/0, "CLIPBOARDUPDATE", "DWMCOMPOSITIONCHANGED", "DWMNCRENDERINGCHANGED",
	/*320*/"DWMCOLORIZATIONCOLORCHANGED", "DWMWINDOWMAXIMIZEDCHANGE", 0, "DWMSENDICONICTHUMBNAIL",
	/*324*/0, 0, "DWMSENDICONICLIVEPREVIEWBITMAP", 0,
};
static_assert(std::string_view(msg_lookup[WM_DWMSENDICONICLIVEPREVIEWBITMAP]) == "DWMSENDICONICLIVEPREVIEWBITMAP");

namespace oui
{
	bool pressed(Key k)
	{
		return (GetKeyState(static_cast<int>(k)) & 0x8000) != 0;
	}


	Input input;

	HWND _focus_wnd = nullptr;

	std::unordered_map<HWND, Window*> windows;

	class SystemWindow;
	LONG WINAPI WindowProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		constexpr int already_down_mask = 1 << 30;

		if (msg < sizeof(msg_lookup) / sizeof(void*))
		{
			if (auto msg_name = msg_lookup[msg])
				switch (msg)
				{
					// ignore these
				case WM_GETICON:
				case WM_MOUSEMOVE:
				case WM_NCHITTEST:
				case WM_SETCURSOR:
					break;
				default: debug::println(msg_name); break;
				}
			else
				debug::println("?? ", debug::hex(msg));
		}
		else
			debug::println("", debug::hex(msg));

		const auto get_point_lparam = [](LPARAM p) { return oui::Point{ float(GET_X_LPARAM(p)), float(GET_Y_LPARAM(p)) }; };
		const auto get_vector_lparam = [](LPARAM p) { return oui::Vector{ float(GET_X_LPARAM(p)), float(GET_Y_LPARAM(p)) }; };

		const auto window = [wnd] 
		{ 
			auto found = windows.find(wnd); 
			return found == windows.end() ? nullptr : found->second; 
		}();

		static bool in_sizemove = false;
		static const auto apply_size = [](HWND wnd, Window& window)
		{
			debug::println(window.size.x, "x", window.size.y);
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
		case WM_SYSCOMMAND:
			if (wParam == SC_KEYMENU)
				return 0;
			break;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if (input.keydown)
				input.keydown(static_cast<Key>(wParam), 
				(lParam&already_down_mask ? PrevKeyState::down : PrevKeyState::up));
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			if (input.keyup)
				input.keyup(static_cast<Key>(wParam));
			break;
		default:
			break;
		}
		return DefWindowProc(wnd, msg, wParam, lParam);
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

		void close() { PostMessage(_wnd, WM_CLOSE, 0, 0); }

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
	void Window::close() 
	{ 
		_renderer->window.close();
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
