#include "mainwindow.hpp"

#include <vector>

pdfv::MainWindow pdfv::MainWindow::mwnd;

void pdfv::MainWindow::aboutBox() noexcept
{
	mwnd.enable(false);
	bool finished{ false };
	auto sz{ dip({ s_cAboutBoxSizeX, s_cAboutBoxSizeY }, dpi) };

	auto hwnd = ::CreateWindowExW(
		0,
		APP_CLASSNAME "About",
		L"About " PRODUCT_NAME,
		WS_POPUPWINDOW | WS_CAPTION,
		mwnd.m_pos.x + (mwnd.m_totalArea.x - sz.x) / 2,
		mwnd.m_pos.y + (mwnd.m_totalArea.y - sz.y) / 2,
		sz.x,
		sz.y,
		mwnd.m_hwnd,
		nullptr,
		nullptr,
		&finished
	);
	if (hwnd != nullptr)
	{
		MSG msg{};
		::ShowWindow(hwnd, SW_SHOW);
		::UpdateWindow(hwnd);

		while (!finished)
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
	}
	mwnd.enable();
	::BringWindowToTop(mwnd.m_hwnd);
}

pdfv::MainWindow::MainWindow() noexcept
{
	auto screen = ::GetDC(nullptr);
	dpi = {
		f(::GetDeviceCaps(screen, LOGPIXELSX)) / 96.0f,
		f(::GetDeviceCaps(screen, LOGPIXELSY)) / 96.0f
	};
	::DeleteDC(screen);

	this->m_menuSize = ::GetSystemMetrics(SM_CYMENU);
	this->m_usableArea = dip({ APP_DEFSIZE_X, APP_DEFSIZE_Y }, dpi);
	this->m_defaultFont = ::CreateFontW(
		dip(15, dpi.y),
		0,
		0,
		0, 
		FW_NORMAL,
		false,
		false,
		false,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		FF_DONTCARE,
		L"Segoe UI"
	);
}

pdfv::MainWindow::~MainWindow() noexcept
{
	if (this->m_defaultFont != nullptr)
	{
		::DeleteObject(this->m_defaultFont);
		this->m_defaultFont = nullptr;
	}
}

[[nodiscard]] bool pdfv::MainWindow::init(HINSTANCE hinst, int argc, wchar_t ** argv) noexcept
{
	this->m_hInst = hinst;
	this->m_argc  = argc;
	this->m_argv  = argv;

	::SetProcessDPIAware();

	// Init common controls
	
	if (!initCC()) [[unlikely]]
	{
		return false;
	}

	// Register classes
	this->m_wcex.style        = CS_HREDRAW | CS_VREDRAW;
	this->m_wcex.lpfnWndProc  = &MainWindow::windowProc;
	this->m_wcex.hInstance    = hinst;
	this->m_wcex.hIcon        = ::LoadIconW(hinst, IDI_APPLICATION);
	this->m_wcex.lpszMenuName = MAKEINTRESOURCEW(IDR_MAINMENU);
	this->m_wcex.hIconSm      = ::LoadIconW(hinst, IDI_APPLICATION);

	if (!registerClasses(this->m_wcex)) [[unlikely]]
	{
		error::lastErr = error::registerclass;
		return false;
	}

	this->m_wcex.lpfnWndProc   = &pdfv::TabObject::tabProc;
	this->m_wcex.lpszClassName = APP_CLASSNAME "Tab";
	this->m_wcex.lpszMenuName  = nullptr;
	this->m_wcex.hbrBackground = ::CreateSolidBrush(RGB(255, 255, 255));
	
	if (!registerClasses(this->m_wcex))
	{
		error::lastErr = error::registertab;
		return false;
	}

	this->m_wcex.lpfnWndProc   = &pdfv::MainWindow::aboutProc;
	this->m_wcex.lpszClassName = APP_CLASSNAME "About";
	this->m_wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW);
	
	if (!registerClasses(this->m_wcex))
	{
		error::lastErr = error::registerabout;
		this->m_helpAvailable = false;
	}

	return true;
}

[[nodiscard]] bool pdfv::MainWindow::run(const wchar_t * fname, int nCmdShow) noexcept
{
	this->m_hwnd = ::CreateWindowExW(
		0,
		APP_CLASSNAME,
		APP_NAME,
		WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		nullptr,
		nullptr,
		this->m_hInst,
		const_cast<wchar_t *>(fname)
	);
	if (this->m_hwnd == nullptr)
	{
		error::lastErr = error::window;
		return false;
	}

	// Add "about" to system menu (menu when caption bar is right-clicked)
	if (this->m_helpAvailable)
	{
		auto hSysMenu = ::GetSystemMenu(this->m_hwnd, false);
		::InsertMenuW(hSysMenu, 5, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
		::InsertMenuW(hSysMenu, 6, MF_BYPOSITION, IDM_HELP_ABOUT, L"&About");
	}
	else
	{
		::EnableMenuItem(::GetMenu(this->m_hwnd), IDM_HELP_ABOUT, MF_DISABLED);
	}
	this->m_hAccelerators = ::LoadAcceleratorsW(this->m_hInst, MAKEINTRESOURCEW(IDR_ACCELERATOR1));

	::ShowWindow(this->m_hwnd, nCmdShow);
	// Good practise
	::UpdateWindow(this->m_hwnd);
	
	return true;
}

int pdfv::MainWindow::msgLoop() const noexcept
{
	MSG msg{};
	BOOL bRet{};

	while ((bRet = ::GetMessageW(&msg, nullptr, 0, 0)) != 0)
	{
		if (bRet == -1) [[unlikely]]
		{
			// Some error happened
			auto lastError = ::GetLastError();
			wchar_t errText[256], temp[256];
			switch (lastError)
			{
			case ERROR_INVALID_WINDOW_HANDLE:
			case ERROR_INVALID_ACCEL_HANDLE:
			case ERROR_INVALID_MENU_HANDLE:
			case ERROR_INVALID_ICON_HANDLE:
			case ERROR_INVALID_CURSOR_HANDLE:
				::FormatMessageW(
					FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					nullptr,
					lastError,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					temp,
					256,
					nullptr
				);
				::swprintf_s(errText, L"%s\nError code: " PTRPRINT, temp, lastError);
				break;
			default:
				::swprintf_s(errText, L"Unknown hard error!\nError code: " PTRPRINT, lastError);
			}
			this->message(errText, MB_ICONERROR | MB_OK);
			return lastError;
		}
		else [[likely]]
		{
			if (!::TranslateAcceleratorW(this->m_hwnd, this->m_hAccelerators, &msg))
			{
				if (!::IsDialogMessageW(this->m_hwnd, &msg))
				{
					::TranslateMessage(&msg);
					::DispatchMessageW(&msg);
				}
			}
		}
	}

	return int(msg.wParam);
}


void pdfv::MainWindow::enable(bool enable) const noexcept
{
	::EnableWindow(this->m_hwnd, enable);
}
void pdfv::MainWindow::setTitle(std::wstring_view newTitle)
{
	this->m_title = newTitle;
	::SetWindowTextW(this->m_hwnd, this->m_title.c_str());
}

int pdfv::MainWindow::message(LPCWSTR message, LPCWSTR msgtitle, UINT type) const noexcept
{
	return ::MessageBoxW(this->m_hwnd, message, msgtitle, type);
}
int pdfv::MainWindow::message(LPCWSTR message, UINT type) const noexcept
{
	return ::MessageBoxW(this->m_hwnd, message, this->m_title.c_str(), type);
}

LRESULT CALLBACK pdfv::MainWindow::windowProc(const HWND hwnd, const UINT uMsg, WPARAM wp, LPARAM lp) noexcept
{
	switch (uMsg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps{};
		[[maybe_unused]] auto hdc = ::BeginPaint(hwnd, &ps);
		
		::EndPaint(hwnd, &ps);
		break;
	}
	case WM_COMMAND:
		switch (LOWORD(wp))
		{
		case IDM_FILE_OPEN:
			if (std::wstring file; mwnd.m_openDialog.open(hwnd, file))
			{
				// Open the PDF
				mwnd.openPdfFile(file);
			}
			break;
		case IDM_FILE_CLOSETAB:
			// Close current tab
			if (mwnd.m_tabs.size() > 1)
			{
				mwnd.m_tabs.remove(mwnd.m_tabs.m_tabindex);
				mwnd.m_tabs.select();
			}
			else
			{
				if (mwnd.m_tabs.getName() == Tabs::defaulttitlepadded)
				{
					::DestroyWindow(hwnd);
				}
				else
				{
					auto it = mwnd.m_tabs.rename(Tabs::defaulttitle);
					it->second->pdfUnload();
					it->updatePDF();
				}
			}
			break;
		case IDM_FILE_EXIT:
			::DestroyWindow(hwnd);
			break;
		case IDM_HELP_ABOUT:
			if (mwnd.m_helpAvailable)
			{
				aboutBox();
			}
			break;
		case IDC_TABULATE:
		{
			ssize_t idx = mwnd.m_tabs.m_tabindex + 1;
			if (idx == ssize_t(mwnd.m_tabs.size()))
			{
				idx = 0;
			}
			mwnd.m_tabs.select(idx);
			break;
		}	
		case IDC_TABULATEBACK:
		{
			ssize_t idx = mwnd.m_tabs.m_tabindex - 1;
			if (idx == -1)
			{
				idx = mwnd.m_tabs.size() - 1;
			}
			mwnd.m_tabs.select(idx);
			break;
		}
		default:
			if (const auto comp = int(wp) - IDM_LIMIT; comp >= 0 && comp < int(mwnd.m_tabs.size()))
			{
				if (mwnd.m_tabs.size() > 1)
				{
					printf("Remove tab %d\n", comp);
					if ((comp < mwnd.m_tabs.m_tabindex) ||
						(mwnd.m_tabs.m_tabindex == int(mwnd.m_tabs.size() - 1)))
					{
						--mwnd.m_tabs.m_tabindex;
					}
					mwnd.m_tabs.remove(comp);
					mwnd.m_tabs.select(mwnd.m_tabs.m_tabindex);
				}
				else
				{
					if (mwnd.m_tabs.getName() == Tabs::defaulttitlepadded)
					{
						::DestroyWindow(hwnd);
					}
					else
					{
						auto it = mwnd.m_tabs.rename(Tabs::defaulttitle);
						it->second->pdfUnload();
						it->updatePDF();
					}
				}
			}
		}
		break;
	case WM_KEYDOWN:
	{
		auto target{ mwnd.m_tabs.m_tabs[mwnd.m_tabs.m_tabindex].tabhandle };
		switch (wp)
		{
		case VK_HOME:
			::SendMessageW(target, WM_VSCROLL, MAKELONG(SB_TOP, 0), 0);
			break;
		case VK_END:
			::SendMessageW(target, WM_VSCROLL, MAKELONG(SB_BOTTOM, 0), 0);
			break;
		case VK_NEXT:
			::SendMessageW(target, WM_VSCROLL, MAKELONG(SB_LINEDOWN, 0), 0);
			break;
		case VK_PRIOR:
			::SendMessageW(target, WM_VSCROLL, MAKELONG(SB_LINEUP, 0), 0);
			break;
		}
		break;
	}
	case WM_MOUSEWHEEL:
	{
		static int delta = 0;
		delta += int(GET_WHEEL_DELTA_WPARAM(wp));

		if (std::abs(delta) >= WHEEL_DELTA)
		{
			POINT p;
			::GetCursorPos(&p);
			auto pt1{ xy<int>{ p.x, p.y } - mwnd.m_pos };
			auto pt2{ mwnd.m_tabs.m_pos  + mwnd.m_tabs.m_offset };
			auto sz { mwnd.m_tabs.m_size - mwnd.m_tabs.m_offset };

			short dir = delta > 0 ? SB_LINEUP : SB_LINEDOWN;
			if (pt1.x >= pt2.x && pt1.x <= (pt2.x + sz.x) &&
				pt1.y >= pt2.y && pt1.y <= (pt2.y + sz.y))
			{
				for (int i = std::abs(delta); i > 0; i -= WHEEL_DELTA)
					::SendMessageW(
						mwnd.m_tabs.m_tabs[mwnd.m_tabs.m_tabindex].tabhandle,
						WM_VSCROLL,
						MAKELONG(dir, 0),
						0
					);
			}
			delta %= ((delta < 0) * -1 + (delta >= 0)) * WHEEL_DELTA;
		}
		break;
	}
	case WM_NOTIFY:
		switch (reinterpret_cast<NMHDR *>(lp)->code)
		{
		case TCN_KEYDOWN:
		{
			auto kd = reinterpret_cast<NMTCKEYDOWN *>(lp);
			::SendMessageW(hwnd, WM_KEYDOWN, kd->wVKey, kd->flags);
			break;
		}
		case TCN_SELCHANGING:
			return FALSE;
		case TCN_SELCHANGE:
			mwnd.m_tabs.selChange();
			break;
		}
		break;
	case WM_MOVE:
		mwnd.m_pos = { LOWORD(lp), HIWORD(lp) };
		break;
	case WM_SIZING:
	{
		auto r = reinterpret_cast<RECT*>(lp);
		if ((r->right - r->left) < mwnd.m_minArea.x)
		{
			switch (wp)
			{
			case WMSZ_BOTTOMRIGHT:
			case WMSZ_RIGHT:
			case WMSZ_TOPRIGHT:
				r->right = r->left + mwnd.m_minArea.x;
				break;
			case WMSZ_BOTTOMLEFT:
			case WMSZ_LEFT:
			case WMSZ_TOPLEFT:
				r->left = r->right - mwnd.m_minArea.x;
				break;
			}
		}
		if ((r->bottom - r->top) < mwnd.m_minArea.y)
		{
			switch (wp)
			{
			case WMSZ_BOTTOM:
			case WMSZ_BOTTOMLEFT:
			case WMSZ_BOTTOMRIGHT:
				r->bottom = r->top + mwnd.m_minArea.y;
				break;
			case WMSZ_TOP:
			case WMSZ_TOPLEFT:
			case WMSZ_TOPRIGHT:
				r->top = r->bottom - mwnd.m_minArea.y;
				break;
			}
		}
		break;
	}
	case WM_SIZE:
		mwnd.m_usableArea = { LOWORD(lp), HIWORD(lp) };
		{
			RECT r{};
			::GetWindowRect(hwnd, &r);
			mwnd.m_totalArea = { r.right - r.left, r.bottom - r.top };
		}
		mwnd.m_border = mwnd.m_totalArea - mwnd.m_usableArea;
		mwnd.m_border.y -= mwnd.m_menuSize;

		mwnd.m_tabs.resize(mwnd.m_usableArea);
		break;
	case WM_CLOSE:
		::DestroyWindow(hwnd);
		mwnd.m_hwnd = nullptr;
		break;
	case WM_DESTROY:
		::PostQuitMessage(pdfv::error::success);
		break;
	case WM_CREATE:	// "Create" co-routine
		// Get border size
		mwnd.m_hwnd = hwnd;
		{
			RECT r1{}, r2{};
			::GetClientRect(hwnd, &r1);
			::GetWindowRect(hwnd, &r2);
			mwnd.m_border = {
				(r2.right  - r2.left) - r1.right,
				(r2.bottom - r2.top)  - r1.bottom
			};
			mwnd.m_totalArea = mwnd.m_usableArea + mwnd.m_border;
			mwnd.m_totalArea.y += mwnd.m_menuSize;
			mwnd.m_minArea = mwnd.m_totalArea;
			mwnd.m_pos = { r2.left, r2.top };
		}
		::MoveWindow(
			hwnd,
			mwnd.m_pos.x, mwnd.m_pos.y,
			mwnd.m_totalArea.x, mwnd.m_totalArea.y,
			true
		);
		{
			wchar_t temp[2048];
			auto len = ::GetWindowTextW(hwnd, temp, 2048);
			mwnd.m_title.assign(temp, len);
		}

		mwnd.m_tabs = Tabs(hwnd, mwnd.getHinst());
		::SendMessageW(
			mwnd.m_tabs.m_tabshwnd,
			WM_SETFONT,
			reinterpret_cast<WPARAM>(mwnd.m_defaultFont),
			false
		);

		mwnd.m_tabs.insert(Tabs::defaulttitle);

		// Open PDF if any
		{
			const wchar_t * fname{
				static_cast<wchar_t *>(reinterpret_cast<CREATESTRUCTW *>(lp)->lpCreateParams)
			};
			if (fname != nullptr && fname[0] != '\0')
			{
				// Open pdf
				std::wstring_view fnv(fname);
				mwnd.openPdfFile(fnv);
				mwnd.m_openDialog.updateName(fnv);
			}
		}
		break;
	case WM_COPYDATA:
	{
		auto receive = reinterpret_cast<const COPYDATASTRUCT *>(lp);
		if (receive != nullptr && receive->dwData > 0)
		{
			// Open pdf
			std::wstring_view fnv(static_cast<wchar_t *>(receive->lpData));
			if (fnv.length() == 0)
			{
				break;
			}
			mwnd.openPdfFile(fnv);
			mwnd.m_openDialog.updateName(fnv);
		}
		break;
	}
	case pdfv::MainWindow::WM_BRINGTOFRONT:
	{
		::ShowWindow(hwnd, SW_RESTORE);
		HWND curWnd = ::GetForegroundWindow();

		DWORD dwMyID  = ::GetWindowThreadProcessId(hwnd, nullptr);
		DWORD dwCurID = ::GetWindowThreadProcessId(curWnd, nullptr);
		::AttachThreadInput(dwCurID, dwMyID, TRUE);
		::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		::SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
		::SetForegroundWindow(hwnd);
		::SetFocus(hwnd);
		::SetActiveWindow(hwnd);
		::AttachThreadInput(dwCurID, dwMyID, FALSE);
		break;
	}
	default:
		return ::DefWindowProcW(hwnd, uMsg, wp, lp);
	}

	return 0;
}

LRESULT CALLBACK pdfv::MainWindow::aboutProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp) noexcept
{
	static bool * finished{};

	switch (uMsg)
	{
	case WM_COMMAND:
		switch (wp)
		{
		case IDOK:
		case IDCANCEL:
			::DestroyWindow(hwnd);
			*finished = true;
			break;
		}
		break;
	case WM_CLOSE:
		::DestroyWindow(hwnd);
		*finished = true;
		break;
	case WM_CREATE:
		finished = reinterpret_cast<bool*>(reinterpret_cast<CREATESTRUCTW*>(lp)->lpCreateParams);
		break;
	default:
		return ::DefWindowProcW(hwnd, uMsg, wp, lp);
	}
	return 0;
}

void pdfv::MainWindow::openPdfFile(std::wstring_view file) noexcept
{
	std::wstring_view fshort;
	for (std::size_t i = file.length() - 1; i > 0; --i)
	{
		if (file[i] == L'\\')
		{
			fshort = file.substr(i + 1);
			break;
		}
	}
	std::vector<pdfv::TabObject>::iterator it;
	if (this->m_tabs.size() == 1 && this->m_tabs.getName() == Tabs::defaulttitlepadded)
	{
		it = this->m_tabs.rename(fshort);
	}
	else
	{
		it = this->m_tabs.insert(fshort);
	}
	
	it->second->pdfLoad(std::wstring(file));
	it->updatePDF();

	this->m_tabs.select();
}
