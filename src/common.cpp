#include "common.hpp"
#include "mainwindow.hpp"

#include <shellapi.h>

[[nodiscard]] RECT pdfv::w::getCliR(HWND hwnd, RECT def) noexcept
{
	RECT r;
	if (::GetClientRect(hwnd, &r)) [[likely]]
	{
		return r;
	}
	else [[unlikely]]
	{
		return def;
	}
}
[[nodiscard]] RECT pdfv::w::getWinR(HWND hwnd, RECT def) noexcept
{
	RECT r;
	if (::GetWindowRect(hwnd, &r)) [[likely]]
	{
		return r;
	}
	else [[unlikely]]
	{
		return def;
	}
}

[[nodiscard]] std::wstring pdfv::w::getWinText(HWND hwnd, const std::wstring & def)
{
	std::wstring str;
	// Default limit for window strings
	str.resize(2048);
	auto len{ ::GetWindowTextW(hwnd, str.data(), str.length() + 1) };
	str.resize(len);
	if (len > 0) [[likely]]
	{
		return str;
	}
	else [[unlikely]]
	{
		return def;
	}
}

void pdfv::w::setFont(HWND hwnd, HFONT hfont, bool redraw) noexcept
{
	::SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(hfont), redraw ? TRUE : FALSE);
}

bool pdfv::w::moveWin(HWND hwnd, int x, int y, bool redraw) noexcept
{
	return ::SetWindowPos(hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | (SWP_NOREDRAW * (!redraw))) ? true : false;
}
bool pdfv::w::moveWin(HWND hwnd, RECT rect, bool redraw) noexcept
{
	return ::MoveWindow(
		hwnd,
		rect.left,              rect.top,
		rect.right - rect.left, rect.bottom - rect.top,
		redraw ? TRUE : FALSE
	) ? true : false;
}

[[nodiscard]] POINT pdfv::w::getCur(POINT def) noexcept
{
	POINT p;
	if (::GetCursorPos(&p)) [[likely]]
	{
		return p;
	}
	else [[unlikely]]
	{
		return def;
	}
}
[[nodiscard]] POINT pdfv::w::getCur(HWND hwnd, POINT def) noexcept
{
	POINT p;
	if (::GetCursorPos(&p) && ::ScreenToClient(hwnd, &p)) [[likely]]
	{
		return p;
	}
	else [[unlikely]]
	{
		return def;
	}
}

bool pdfv::w::redraw(HWND hwnd, bool erase) noexcept
{
	return ::InvalidateRect(hwnd, nullptr, erase ? TRUE : FALSE) ? true : false; 
}

bool pdfv::w::reorder(HWND hwnd, HWND hbefore, bool redraw) noexcept
{
	return ::SetWindowPos(hwnd, hbefore, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | (SWP_NOREDRAW * (!redraw))) ? true : false;
}

bool pdfv::w::resize(HWND hwnd, int x, int y, bool redraw) noexcept
{
	return ::SetWindowPos(hwnd, nullptr, 0, 0, x, y, SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER | (SWP_NOREDRAW * (!redraw))) ? true : false;
}

void pdfv::w::openWeb(LPCWSTR url) noexcept
{
	::ShellExecuteW(nullptr, L"open", url, nullptr, nullptr, SW_SHOWNORMAL);
}

bool pdfv::w::status::setParts(HWND statusbar, const std::vector<int> & edges) noexcept
{
	// Number of edges can't be greater than 256
	if (edges.size() > 256) [[unlikely]]
	{
		return false;
	}
	return ::SendMessageW(statusbar, SB_SETPARTS, edges.size(), reinterpret_cast<LPARAM>(edges.data())) ? true : false;
}
bool pdfv::w::status::setText(HWND statusbar, int idx, DrawOp drawop, LPCWSTR str) noexcept
{
	return ::SendMessageW(statusbar, SB_SETTEXT, idx | int(drawop), reinterpret_cast<LPARAM>(str)) ? true : false;
}




[[nodiscard]] pdfv::ArgVecT pdfv::getArgs(LPWSTR cmdLine, int & argc) noexcept
{
	DEBUGPRINT("pdfv::getArgs(%p, %p)\n", static_cast<void *>(cmdLine), static_cast<void *>(&argc));
	wchar_t ** argv{ ::CommandLineToArgvW(cmdLine, &argc) };
	if (argv == nullptr) [[unlikely]]
	{
		argc = 0;
	}

	return ArgVecT(argv);
}

[[nodiscard]] bool pdfv::initCC() noexcept
{
	INITCOMMONCONTROLSEX iccex{};
	iccex.dwSize = sizeof iccex;
	iccex.dwICC  = ICC_WIN95_CLASSES;

	if (!::InitCommonControlsEx(&iccex)) [[unlikely]]
	{
		error::lastErr = error::commoncontrols;
		return false;
	}

	iccex.dwICC = ICC_TAB_CLASSES;
	if (!::InitCommonControlsEx(&iccex)) [[unlikely]]
	{
		error::lastErr = error::commoncontrols;
		return false;
	}

	return true;
}

// Forward-declare certain template types for faster compiling when actually used
template class pdfv::xy<int>;
template class pdfv::xy<float>;

pdfv::xy<float> pdfv::dpi{ 1.0f, 1.0f };

[[nodiscard]] ATOM pdfv::registerClasses(WNDCLASSEXW & wcex) noexcept
{
	DEBUGPRINT("pdfv::RegisterClasses(%p)\n", static_cast<void *>(&wcex));
	if (wcex.cbSize == 0)
	{
		wcex.cbSize = sizeof(WNDCLASSEXW);
	}
	if (wcex.style == 0)
	{
		wcex.style = CS_HREDRAW | CS_VREDRAW;
	}
	if (wcex.hbrBackground == nullptr)
	{
		wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW);
	}
	if (wcex.hCursor == nullptr)
	{
		wcex.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
	}
	if (wcex.hIcon == nullptr)
	{
		wcex.hIcon = ::LoadIconW(nullptr, IDI_APPLICATION);
	}
	if (wcex.hIconSm == nullptr)
	{
		wcex.hIconSm = wcex.hIcon;
	}
	if (wcex.lpszClassName == nullptr)
	{
		wcex.lpszClassName = APP_CLASSNAME;
	}

	return ::RegisterClassExW(&wcex);
}

namespace pdfv
{
	static constexpr UINT WM_MESSAGE{ WM_USER };
	static bool AskProc_finished{ false };

	LRESULT CALLBACK askProc(const HWND hwnd, const UINT uMsg, WPARAM wp, LPARAM lp) noexcept
	{
		static wchar_t * textdata{ nullptr };
		static HWND textbox{ nullptr }, messagebox{ nullptr };
		
		switch (uMsg)
		{
		case WM_COMMAND:
			switch (wp)
			{
			case IDOK:
				if (textdata != nullptr && textbox != nullptr) [[likely]]
				{
					::GetWindowTextW(textbox, textdata, 2048);
				}
				[[fallthrough]];
			case IDCANCEL:
				::DestroyWindow(hwnd);
				AskProc_finished = true;
				break;
			}
			break;
		case WM_CLOSE:
			::DestroyWindow(hwnd);
			AskProc_finished = true;
			break;
		case WM_CREATE:
		{
			auto button1{ ::CreateWindowExW(
				0,
				L"button",
				L"&OK",
				WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP,
				dip(153, dpi.x),
				dip(100, dpi.y),
				dip(80 , dpi.x),
				dip(24 , dpi.y),
				hwnd,
				reinterpret_cast<HMENU>(IDOK),
				nullptr,
				nullptr
			) };
			auto button2{ ::CreateWindowExW(
				0,
				L"button",
				L"&Cancel",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				dip(240, dpi.y),
				dip(100, dpi.y),
				dip(80 , dpi.x),
				dip(24 , dpi.x),
				hwnd,
				reinterpret_cast<HMENU>(IDCANCEL),
				nullptr,
				nullptr
			) };
			messagebox = ::CreateWindowExW(
				0,
				L"static",
				L"",
				WS_CHILD | WS_VISIBLE,
				dip(10,  dpi.x), dip(10, dpi.y),
				dip(310, dpi.x), dip(50, dpi.y),
				hwnd,
				nullptr,
				nullptr,
				nullptr
			);
			textbox = ::CreateWindowExW(
				WS_EX_CLIENTEDGE,
				L"edit",
				L"",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				dip(10,  dpi.x), dip(65, dpi.y),
				dip(310, dpi.x), dip(22, dpi.y),
				hwnd,
				nullptr,
				nullptr,
				nullptr
			);
			auto hfont{ window.getDefFont() };
			w::setFont(button1,    hfont, true);
			w::setFont(button2,    hfont, true);
			w::setFont(messagebox, hfont, true);
			w::setFont(textbox,    hfont, true);

			w::reorder(button1, textbox);
			w::reorder(button2, button1);
			w::reorder(textbox, button2);

			auto s1{ make_xy(w::getCliR(hwnd)) };
			auto s2{ make_xy(w::getWinR(hwnd)) };
			w::resize(hwnd, 2 * s2.x - s1.x, 2 * s2.y - s1.y, true);

			textdata = static_cast<wchar_t *>(reinterpret_cast<CREATESTRUCTW *>(lp)->lpCreateParams);
			break;
		}
		case pdfv::WM_MESSAGE:
			::SetWindowTextW(messagebox, reinterpret_cast<wchar_t *>(lp));
			break;
		default:
			return ::DefWindowProcW(hwnd, uMsg, wp, lp);
		}

		return 0;
	}
}

[[nodiscard]] std::wstring pdfv::askInfo(std::wstring_view message, std::wstring_view title) noexcept
{
	DEBUGPRINT("pdfv::askInfo(%p, %p)\n", static_cast<const void *>(message.data()), static_cast<const void *>(title.data()));

	WNDCLASSEXW wc{ window.getWinClass() };
	wc.lpfnWndProc   = &pdfv::askProc;
	wc.lpszClassName = L"AskInfoClass";
	wc.lpszMenuName  = nullptr;

	if (!registerClasses(wc)) [[unlikely]]
	{
		return {};
	}
	window.enable(false);
	AskProc_finished = false;

	wchar_t temp[2048]{};
	auto hwnd = ::CreateWindowExW(
		0,
		wc.lpszClassName,
		title.data(),
		WS_POPUPWINDOW | WS_CAPTION,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		dip(330, dpi.x),
		dip(142, dpi.y),
		window.getHandle(),
		nullptr,
		nullptr,
		reinterpret_cast<LPVOID>(temp)
	);
	if (hwnd == nullptr) [[unlikely]]
	{
		window.enable();
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return {};
	}
	::SendMessageW(hwnd, WM_MESSAGE, 0, reinterpret_cast<LPARAM>(message.data()));
	
	::ShowWindow(hwnd, SW_SHOW);
	::UpdateWindow(hwnd);

	MSG msg{};
	while (!AskProc_finished)
	{
		if (::GetMessageW(&msg, nullptr, 0, 0))
		{
			if (!::IsDialogMessageW(hwnd, &msg))
			{
				::TranslateMessage(&msg);
				::DispatchMessageW(&msg);
			}
		}
	}
	::DestroyWindow(hwnd);

	window.enable();
	::UnregisterClassW(wc.lpszClassName, wc.hInstance);
	return temp;
}

[[nodiscard]] std::wstring pdfv::utf::conv(std::string_view str)
{
	DEBUGPRINT("pdfv::utf::conv(string_view %p)\n", static_cast<const void *>(str.data()));
	
	auto len{ ::MultiByteToWideChar(
		CP_UTF8,
		MB_PRECOMPOSED,
		str.data(),
		int(str.size()) + 1,
		nullptr,
		0
	) };

	std::wstring wstr;
	wstr.resize(len);

	::MultiByteToWideChar(
		CP_UTF8,
		MB_PRECOMPOSED,
		str.data(),
		int(str.size()) + 1,
		wstr.data(),
		len
	);

	return wstr;
}
[[nodiscard]] std::string pdfv::utf::conv(std::wstring_view wstr)
{
	DEBUGPRINT("pdfv::utf::conv(wstring_view %p)\n", static_cast<const void *>(wstr.data()));

	auto len{ ::WideCharToMultiByte(
		CP_UTF8,
		0,
		wstr.data(),
		int(wstr.size()) + 1,
		nullptr,
		0,
		nullptr,
		nullptr
	) };

	std::string str;
	str.resize(len);

	::WideCharToMultiByte(
		CP_UTF8,
		0,
		wstr.data(),
		int(wstr.size()) + 1,
		str.data(),
		len,
		nullptr,
		nullptr
	);

	return str;
}
