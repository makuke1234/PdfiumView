#include "common.hpp"
#include "main_window.hpp"

template class pdfv::xy<int>;
template class pdfv::xy<float>;

pdfv::xy<float> pdfv::dpi{ 1.0f, 1.0f };

[[nodiscard]] ATOM pdfv::RegisterClasses(WNDCLASSEXW& wcex) noexcept
{
	if (wcex.cbSize == 0)
		wcex.cbSize = sizeof(WNDCLASSEXW);
	if (wcex.style == 0)
		wcex.style = CS_HREDRAW | CS_VREDRAW;
	if (wcex.hbrBackground == nullptr)
		wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW);
	if (wcex.hCursor == nullptr)
		wcex.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
	if (wcex.hIcon == nullptr)
		wcex.hIcon = ::LoadIconW(nullptr, IDI_APPLICATION);
	if (wcex.hIconSm == nullptr)
		wcex.hIconSm = wcex.hIcon;
	if (wcex.lpszClassName == nullptr)
		wcex.lpszClassName = APP_CLASSNAME;

	return ::RegisterClassExW(&wcex);
}

namespace pdfv {
	static constexpr UINT WM_MESSAGE = WM_USER + 1;
	static bool AskProc_finished = false;

	LRESULT CALLBACK AskProc(const HWND hwnd, const UINT uMsg, WPARAM wp, LPARAM lp) noexcept
	{
		static wchar_t *textdata{};
		static HWND textbox{}, messagebox{};
		
		switch (uMsg) {
		case WM_COMMAND:
			switch (wp) {
			case IDOK:
				if (textdata != nullptr && textbox != nullptr) {
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
			auto button1 = ::CreateWindowExW(
				0,
				L"button",
				L"&OK",
				WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP,
				DPI(153, dpi.x),
				DPI(100, dpi.y),
				DPI(80 , dpi.x),
				DPI(24 , dpi.y),
				hwnd,
				reinterpret_cast<HMENU>(IDOK),
				nullptr,
				nullptr
			);
			auto button2 = ::CreateWindowExW(
				0,
				L"button",
				L"&Cancel",
				WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				DPI(240, dpi.y),
				DPI(100, dpi.y),
				DPI(80 , dpi.x),
				DPI(24 , dpi.x),
				hwnd,
				reinterpret_cast<HMENU>(IDCANCEL),
				nullptr,
				nullptr
			);
			messagebox = ::CreateWindowExW(
				0,
				L"static",
				L"",
				WS_CHILD | WS_VISIBLE,
				DPI(10 , dpi.x),
				DPI(10 , dpi.y),
				DPI(310, dpi.x),
				DPI(50 , dpi.y),
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
				DPI(10 , dpi.x),
				DPI(65 , dpi.y),
				DPI(310, dpi.x),
				DPI(22 , dpi.y),
				hwnd,
				nullptr,
				nullptr,
				nullptr
			);
			auto font = reinterpret_cast<WPARAM>(main_window::mwnd.GetDefaultFont());
			::SendMessageW(button1   , WM_SETFONT, font, true);
			::SendMessageW(button2   , WM_SETFONT, font, true);
			::SendMessageW(messagebox, WM_SETFONT, font, true);
			::SendMessageW(textbox   , WM_SETFONT, font, true);

			::SetWindowPos(button1, textbox, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			::SetWindowPos(button2, button1, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			::SetWindowPos(textbox, button2, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

			RECT r1, r2;
			::GetClientRect(hwnd, &r1);
			::GetWindowRect(hwnd, &r2);
			::MoveWindow(
				hwnd,
				r1.left, r1.top,
				2 * (r2.right - r2.left) - r1.right,
				2 * (r2.bottom - r2.top) - r1.bottom,
				true
			);

			textdata = reinterpret_cast<wchar_t*>(reinterpret_cast<CREATESTRUCTW*>(lp)->lpCreateParams);
			break;
		}
		case pdfv::WM_MESSAGE:
			::SetWindowTextW(messagebox, reinterpret_cast<wchar_t*>(lp));
			break;
		default:
			return ::DefWindowProcW(hwnd, uMsg, wp, lp);
		}
		return 0;
	}
}

[[nodiscard]] std::wstring pdfv::AskInfo(std::wstring_view message, std::wstring_view title) noexcept
{
	WNDCLASSEXW wc{ main_window::mwnd.GetWindowClass() };
	wc.lpfnWndProc   = &pdfv::AskProc;
	wc.lpszClassName = L"AskInfoClass";
	wc.lpszMenuName  = nullptr;

	if (!RegisterClasses(wc)) {
		return {};
	}
	main_window::mwnd.Enable(false);
	AskProc_finished = false;
	wchar_t temp[2048];
	temp[0] = 0;
	// Good practise
	temp[2047] = 0;
	auto hwnd = ::CreateWindowExW(
		0,
		wc.lpszClassName,
		title.data(),
		WS_POPUPWINDOW | WS_CAPTION,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		DPI(330, dpi.x),
		DPI(142, dpi.y),
		main_window::mwnd.GetHWND(),
		nullptr,
		nullptr,
		reinterpret_cast<LPVOID>(temp)
	);
	if (hwnd == nullptr) {
		main_window::mwnd.Enable();
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return {};
	}
	::SendMessageW(hwnd, WM_MESSAGE, 0, reinterpret_cast<LPARAM>(message.data()));
	
	::ShowWindow(hwnd, SW_SHOW);
	::UpdateWindow(hwnd);

	MSG msg{};
	while (!AskProc_finished) {
		if (::GetMessageW(&msg, nullptr, 0, 0)) {
			if (!::IsDialogMessageW(hwnd, &msg)) {
				::TranslateMessage(&msg);
				::DispatchMessageW(&msg);
			}
		}
	}
	::DestroyWindow(hwnd);

	main_window::mwnd.Enable();
	::UnregisterClassW(wc.lpszClassName, wc.hInstance);
	return temp;
}

[[nodiscard]] std::wstring pdfv::Convert(std::string_view input)
{
	std::unique_ptr<wchar_t> temp(new wchar_t[input.length() + 1]);
	::MultiByteToWideChar(
		CP_UTF8,
		MB_COMPOSITE,
		input.data(),
		input.length(),
		temp.get(),
		input.length()
	);
	temp.get()[input.length()] = 0;

	return temp.get();
}
[[nodiscard]] std::string pdfv::Convert(std::wstring_view input)
{
	std::unique_ptr<char> temp(new char[input.length() + 1]);
	::WideCharToMultiByte(
		CP_UTF8,
		0,
		input.data(),
		input.length(),
		temp.get(),
		input.length(),
		nullptr,
		nullptr
	);
	temp.get()[input.length()] = 0;

	return temp.get();
}
