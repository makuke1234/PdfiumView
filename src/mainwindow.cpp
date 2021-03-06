#include "mainwindow.hpp"

#include <vector>


[[nodiscard]] bool pdfv::MainWindow::intersectsTabClose() noexcept
{
	auto p{ w::getCur(this->getHandle()) };

	for (std::size_t i = 0; i < this->m_tabs->size(); ++i)
	{
		RECT r;
		TabCtrl_GetItemRect(this->m_tabs->getTabsHandle(), i, &r);
		auto closeR{ Tabs::s_calcCloseButton(r) };

		if (p.x >= closeR.left && p.x <= closeR.right &&
			p.y >= closeR.top  && p.y <= closeR.bottom
		)
		{
			this->m_highlightedIdx = i;
			return true;
		}
	}

	return false;
}

void pdfv::MainWindow::showAboutBox() noexcept
{
	DEBUGPRINT("pdfv::MainWindow::aboutBox()\n");
	::DialogBoxParamW(
		this->m_hInst,
		MAKEINTRESOURCEW(IDD_ABOUTDIALOG),
		this->m_hwnd,
		&MainWindow::aboutProc,
		reinterpret_cast<LPARAM>(this)
	);
}

void pdfv::MainWindow::setStatusParts() const noexcept
{
	auto w{ this->m_usableArea.x };
	w::status::setParts(this->m_statushwnd, { w - dip(210, dpi.x), w - dip(120, dpi.x), w - dip(17, dpi.x) });
}

pdfv::MainWindow::MainWindow() noexcept
{
	DEBUGPRINT("pdfv::MainWindow::MainWindow()\n");
	
	auto screen{ ::GetDC(nullptr) };
	dpi = {
		f32(::GetDeviceCaps(screen, LOGPIXELSX)) / 96.0f,
		f32(::GetDeviceCaps(screen, LOGPIXELSY)) / 96.0f
	};
	::DeleteDC(screen);

	this->m_menuSize = ::GetSystemMetrics(SM_CYMENU);
	this->m_usableArea = dip({ APP_DEFSIZE_X, APP_DEFSIZE_Y }, dpi);
	this->m_defaultFont = ::CreateFontW(
		dipfont(9, dpi.y),
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

	this->m_defaultFontBold = ::CreateFontW(
		dipfont(9, dpi.y),
		0,
		0,
		0,
		FW_BOLD,
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
	DEBUGPRINT("pdfv::MainWindow::~MainWindow()\n");

	// Kill moving thread
	if (this->m_moveThread != nullptr) [[likely]]
	{
		this->m_moveKillSwitch = true;
		::WaitForSingleObject(this->m_moveThread, 1000);
		::TerminateThread(this->m_moveThread, 0);
		this->m_moveThread = nullptr;
	}

	if (this->m_hwnd != nullptr) [[unlikely]]
	{
		::DestroyWindow(this->m_hwnd);
		this->m_hwnd = nullptr;
	}
}

[[nodiscard]] bool pdfv::MainWindow::init(HINSTANCE hinst, int argc, wchar_t ** argv) noexcept
{
	DEBUGPRINT("pdfv::MainWindow::init(%p, %d, %p)\n", static_cast<void *>(hinst), argc, static_cast<void *>(argv));
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

	if (!registerClass(this->m_wcex)) [[unlikely]]
	{
		error::lastErr = error::registerclass;
		return false;
	}

	this->m_wcex.style         = 0;
	this->m_wcex.lpfnWndProc   = &pdfv::Tabs::tabsCanvasProcHub;
	this->m_wcex.lpszClassName = pdfv::Tabs::c_className;
	this->m_wcex.lpszMenuName  = nullptr;
	
	if (!registerClass(this->m_wcex)) [[unlikely]]
	{
		error::lastErr = error::registertabscanvas;
		return false;
	}

	return true;
}

[[nodiscard]] bool pdfv::MainWindow::run(const wchar_t * fname, int nCmdShow) noexcept
{
	DEBUGPRINT("pdfv::MainWindow::run(%p, %s, %d)\n", static_cast<const void *>(fname), fname, nCmdShow);
	std::pair<MainWindow *, const wchar_t *> initPair{ this, fname };
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
		&initPair
	);
	if (this->getHandle() == nullptr) [[unlikely]]
	{
		error::lastErr = error::window;
		return false;
	}

	// Add "about" to system menu (menu when caption bar is right-clicked)
	if (this->m_helpAvailable) [[likely]]
	{
		auto hSysMenu{ ::GetSystemMenu(this->getHandle(), false) };
		::InsertMenuW(hSysMenu, 5, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
		::InsertMenuW(hSysMenu, 6, MF_BYPOSITION, IDM_HELP_ABOUT, L"&About");
	}
	else [[unlikely]]
	{
		::EnableMenuItem(::GetMenu(this->getHandle()), IDM_HELP_ABOUT, MF_DISABLED);
	}
	this->m_hAccelerators = ::LoadAcceleratorsW(this->m_hInst, MAKEINTRESOURCEW(IDR_ACCELERATOR1));

	::ShowWindow(this->getHandle(), nCmdShow);
	// Good practise
	::UpdateWindow(this->getHandle());

	return true;
}

int pdfv::MainWindow::msgLoop() const noexcept
{
	DEBUGPRINT("pdfv::MainWindow::msgLoop()\n");
	MSG msg{};
	BOOL bRet{};

	while ((bRet = ::GetMessageW(&msg, nullptr, 0, 0)) != 0)
	{
		if (bRet == -1) [[unlikely]]
		{
			// Some error happened
			auto lastError{ ::GetLastError() };
			constexpr auto maxErr{ 256 };

			wchar_t errText[maxErr], temp[maxErr];
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
					maxErr,
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
			if (!::TranslateAcceleratorW(this->getHandle(), this->m_hAccelerators, &msg))
			{
				if (!::IsDialogMessageW(this->getHandle(), &msg))
				{
					::TranslateMessage(&msg);
					::DispatchMessageW(&msg);
				}
			}
		}
	}

	return int(msg.wParam);
}

void pdfv::MainWindow::close() const noexcept
{
	if (this->getHandle() != nullptr) [[likely]]
	{
		::SendMessageW(this->getHandle(), WM_CLOSE, 0, 0);
	}
}


void pdfv::MainWindow::enable(bool enable) const noexcept
{
	DEBUGPRINT("pdfv::MainWindow::enable(%d)\n", enable);
	::EnableWindow(this->getHandle(), enable);
}
void pdfv::MainWindow::setTitle(std::wstring_view newTitle)
{
	DEBUGPRINT("pdfv::MainWindow::setTitle(%p)\n", static_cast<const void *>(newTitle.data()));
	this->m_title = newTitle;
	::SetWindowTextW(this->getHandle(), this->m_title.c_str());
}

int pdfv::MainWindow::message(LPCWSTR message, LPCWSTR msgtitle, UINT type) const noexcept
{
	DEBUGPRINT("pdfv::MainWindow::message(%p, %p, %u)\n", static_cast<const void *>(message), static_cast<const void *>(msgtitle), type);
	return ::MessageBoxW(this->getHandle(), message, msgtitle, type);
}
int pdfv::MainWindow::message(LPCWSTR message, UINT type) const noexcept
{
	DEBUGPRINT("pdfv::MainWindow::message(%p, %u)\n", static_cast<const void *>(message), type);
	return ::MessageBoxW(this->getHandle(), message, this->m_title.c_str(), type);
}

LRESULT CALLBACK pdfv::MainWindow::windowProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp) noexcept
{
	MainWindow * self{ nullptr };
	if (uMsg == WM_CREATE) [[unlikely]]
	{
		auto cs{ reinterpret_cast<CREATESTRUCTW *>(lp) };
		auto vals = static_cast<std::pair<MainWindow *, const wchar_t *> *>(cs->lpCreateParams);
		self = vals->first;
		w::setPtr(hwnd, self);
		cs->lpCreateParams = const_cast<wchar_t *>(vals->second);
	}
	else [[likely]]
	{
		self = w::getPtr<MainWindow *>(hwnd);
	}

	if (self != nullptr) [[likely]]
	{
		return self->intWindowProc(hwnd, uMsg, wp, lp);
	}
	else [[unlikely]]
	{
		return ::DefWindowProcW(hwnd, uMsg, wp, lp);
	}
}
LRESULT pdfv::MainWindow::intWindowProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp) noexcept
{
	switch (uMsg)
	{
	case WM_DRAWITEM:
		return this->wOnDrawItem(lp);
	case WM_ERASEBKGND:
		return TRUE;
	case WM_COMMAND:
		this->wOnCommand(wp);
		break;
	case WM_KEYDOWN:
		this->m_keyDown[wp] = true;
		[[fallthrough]];
	case WM_SPECIALKEYDOWN:
		this->wOnKeydown(wp);
		break;
	case WM_KEYUP:
		DEBUGPRINT("Key up %d!\n", wp);
		this->m_keyDown[wp] = false;
		break;
	case WM_MOUSEWHEEL:
		this->wOnMousewheel(wp);
		break;
	case WM_LBUTTONDOWN:
		this->wOnLButtonDown(wp, lp);
		break;
	case WM_LBUTTONUP:
		this->wOnLButtonUp(wp, lp);
		break;
	case WM_NOTIFY:
		return this->wOnNotify(lp);
	case WM_MOVE:
		this->wOnMove(lp);
		break;
	case WM_SIZING:
		this->wOnSizing(wp, lp);
		break;
	case WM_SIZE:
		this->wOnSize();
		break;
	case WM_CLOSE:
		::DestroyWindow(hwnd);
		this->m_hwnd = nullptr;
		break;
	case WM_DESTROY:
		::PostQuitMessage(pdfv::error::success);
		break;
	case WM_CREATE:
		this->wOnCreate(hwnd, lp);
		break;
	case WM_COPYDATA:
		this->wOnCopydata(lp);
		break;
	case WM_MOUSEMOVE:
	case pdfv::MainWindow::WM_TABMOUSEMOVE:
		this->wOnTabMouseMove(wp, lp);
		break;
	case pdfv::MainWindow::WM_BRINGTOFRONT:
		this->wOnBringToFront();
		break;
	default:
		return ::DefWindowProcW(hwnd, uMsg, wp, lp);
	}

	return 0;
}

LRESULT pdfv::MainWindow::wOnDrawItem(LPARAM lp) noexcept
{
	auto dis{ reinterpret_cast<DRAWITEMSTRUCT *>(lp) };
	DEBUGPRINT("pdfv::MainWindow::wOnDrawItem(%p)\n", static_cast<void *>(dis));
	
	if (dis->hwndItem == this->m_tabs->m_tabshwnd)
	{
		// Do double-buffering
		auto w{ dis->rcItem.right  - dis->rcItem.left };
		auto h{ dis->rcItem.bottom - dis->rcItem.top  };

		auto hdc{ ::CreateCompatibleDC(dis->hDC) };
		auto hbmBuf{ ::CreateCompatibleBitmap(dis->hDC, dis->rcItem.right, dis->rcItem.bottom) };
		auto hbmOld{ ::SelectObject(hdc, hbmBuf) };

		::FillRect(hdc, &dis->rcItem, reinterpret_cast<HBRUSH>(COLOR_WINDOW));
		::SetBkMode(hdc, TRANSPARENT);

		// Currently selected tab
		auto cursel{ TabCtrl_GetCurSel(dis->hwndItem) };

		if (int(dis->itemID) == cursel)
		{
			::SetTextColor(hdc, RGB(0, 0, 0));
		}
		else
		{
			::SetTextColor(hdc, RGB(127, 127, 127));
		}

		auto r{ dis->rcItem };

		// Draw text
		::SelectObject(hdc, this->getDefFont());
		const auto & item{ this->m_tabs->m_tabs[dis->itemID] };
		::DrawTextW(hdc, item.first.c_str(), item.first.length(), &r, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

		// Draw close button last
		auto closeR{ Tabs::s_calcCloseButton(r) };
		
		// Check mouse position

		::FillRect(
			hdc,
			&closeR,
			(this->m_highlighted && (this->m_highlightedIdx == dis->itemID)) ?
				(this->m_closeButtonDown ? this->m_darkRedBrush.get() : this->m_redBrush.get()) :
				 this->m_brightRedBrush.get()
		);

		// Draw text on top of close button
		::SetTextColor(hdc, RGB(255, 255, 255));
		::SelectObject(hdc, this->getDefFontBold());
		::DrawTextW(hdc, L"X", 1, &closeR, DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_NOCLIP);
		
		// Double-buffering action

		::BitBlt(dis->hDC, dis->rcItem.left, dis->rcItem.top, w, h, hdc, dis->rcItem.left, dis->rcItem.top, SRCCOPY);
		
		::SelectObject(hdc, hbmOld);
		::DeleteObject(hbmBuf);
		::DeleteDC(hdc);

		return TRUE;
	}
	return FALSE;
}
void pdfv::MainWindow::wOnCommand(WPARAM wp) noexcept
{
	DEBUGPRINT("pdfv::MainWindow::onCommand(%u)\n", wp);

	switch (LOWORD(wp))
	{
	case IDM_FILE_OPEN:
		if (std::wstring file; this->m_openDialog.open(this->getHandle(), file))
		{
			// Open the PDF
			this->openPdfFile(file);
		}
		break;
	case IDM_FILE_CLOSETAB:
		if (this->m_keyDown[L'W'] && HIWORD(wp) == 1)
		{
			break;
		}
		this->m_keyDown[L'W'] = true;
		// Close current tab
		::SendMessageW(this->getHandle(), WM_COMMAND, IDM_LIMIT + this->m_tabs->m_tabindex, 0);
		break;
	case IDM_FILE_EXIT:
		this->close();
		break;
	case IDM_HELP_ABOUT:
		if (this->m_helpAvailable)
		{
			this->showAboutBox();
		}
		break;
	case IDC_TABULATE:
	{
		ssize_t idx{ this->m_tabs->m_tabindex + 1 };
		if (idx == ssize_t(this->m_tabs->size()))
		{
			idx = 0;
		}
		this->m_tabs->select(idx);
		break;
	}	
	case IDC_TABULATEBACK:
	{
		ssize_t idx{ this->m_tabs->m_tabindex - 1 };
		if (idx == -1)
		{
			idx = this->m_tabs->size() - 1;
		}
		this->m_tabs->select(idx);
		break;
	}
	case IDC_ZOOMRESET:
		::SendMessageW(this->m_tabs->getCanvasHandle(), Tabs::WM_ZOOMRESET, 0, 0);
		break;
	case IDC_ZOOMPLUS:
		::SendMessageW(this->m_tabs->getCanvasHandle(), Tabs::WM_ZOOM, WHEEL_DELTA / 10, 0);
		break;
	case IDC_ZOOMMINUS:
		::SendMessageW(this->m_tabs->getCanvasHandle(), Tabs::WM_ZOOM, -WHEEL_DELTA / 10, 0);
		break;
	default:
		if (const auto comp{ int(LOWORD(wp)) - IDM_LIMIT }; comp >= 0 && comp < int(this->m_tabs->size()))
		{
			if (this->m_tabs->size() > 1)
			{
				printf("Remove tab %d\n", comp);
				if ((comp < this->m_tabs->m_tabindex) ||
					(this->m_tabs->m_tabindex == int(this->m_tabs->size() - 1)))
				{
					--this->m_tabs->m_tabindex;
				}
				this->m_tabs->remove(comp);
				this->m_tabs->select(this->m_tabs->m_tabindex);
			}
			else
			{
				if (this->m_tabs->getName() == Tabs::defaulttitlepadded)
				{
					this->close();
				}
				else
				{
					auto it = this->m_tabs->rename(Tabs::defaulttitle);
					(it)->second.pdfUnload();
					this->m_tabs->redrawTabs();
				}
			}
		}
	}
}
void pdfv::MainWindow::wOnKeydown(WPARAM wp) noexcept
{
	DEBUGPRINT("pdfv::MainWindow::wOnKeyDown(%u)\n", wp);
	auto target{ this->m_tabs->getCanvasHandle() };
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
}
void pdfv::MainWindow::wOnMousewheel(WPARAM wp) noexcept
{
	DEBUGPRINT("pdfv::MainWindow::wOnMousewheel(%u)\n", wp);
	static int delta{ 0 };

	auto p{ w::getCur() };
	auto pt1{ xy<int>{ p.x, p.y } - this->m_pos };
	auto pt2{ this->m_tabs->m_pos  + this->m_tabs->m_offset };
	auto sz { this->m_tabs->m_size - this->m_tabs->m_offset };
	if (!(pt1.x >= pt2.x && pt1.x <= (pt2.x + sz.x) &&
		pt1.y >= pt2.y && pt1.y <= (pt2.y + sz.y)))
	{
		// Out of bounds
		return;
	}

	auto newdelta{ int(GET_WHEEL_DELTA_WPARAM(wp)) };
	DEBUGPRINT("new wheel delta %d\n", newdelta);
	bool controlDown{ (LOWORD(wp) & MK_CONTROL) != 0 };
	
	if (controlDown)
	{
		// Reset scrolling if zooming is applied
		delta = 0;
		::SendMessageW(
			this->m_tabs->getCanvasHandle(),
			Tabs::WM_ZOOM,
			newdelta,
			0
		);
	}
	else
	{
		delta += newdelta;
		auto dir{ delta > 0 ? SB_LINEUP : SB_LINEDOWN };

		for (int i = std::abs(delta); i >= WHEEL_DELTA; i -= WHEEL_DELTA)
		{
			::SendMessageW(
				this->m_tabs->getCanvasHandle(),
				WM_VSCROLL,
				MAKELONG(dir, 0),
				0
			);
			delta += (dir == SB_LINEDOWN) ? WHEEL_DELTA : -WHEEL_DELTA;
		}
	}
	DEBUGPRINT("pdfv::MainWindow::wOnMouseWheel end!!!!\n");
}
void pdfv::MainWindow::wOnLButtonDown([[maybe_unused]] WPARAM wp, [[maybe_unused]] LPARAM lp) noexcept
{
	if (this->m_highlighted)
	{
		this->m_closeButtonDown = true;
		this->m_tabs->redrawTabs();
	}
}
void pdfv::MainWindow::wOnLButtonUp([[maybe_unused]] WPARAM wp, [[maybe_unused]] LPARAM lp) noexcept
{
	if (this->m_highlighted && this->m_closeButtonDown)
	{
		// Click close button
		::SendMessageW(this->getHandle(), WM_COMMAND, IDM_LIMIT + this->m_highlightedIdx, 0);
		this->m_tabs->redrawTabs();
	}
	this->m_closeButtonDown = false;
}
void pdfv::MainWindow::wOnTabMouseMove([[maybe_unused]] WPARAM wp, [[maybe_unused]] LPARAM lp) noexcept
{
	DEBUGPRINT("pdfv::MainWindow::wOnTabMouseMove(%u, %u)\n", wp, lp);
	
	if (auto high{ this->intersectsTabClose() }; high != this->m_highlighted)
	{
		this->m_highlighted = high;
		this->m_tabs->redrawTabs();
	}
}
LRESULT pdfv::MainWindow::wOnNotify(LPARAM lp) noexcept
{
	DEBUGPRINT("pdfv::MainWIndow::wOnNotify(%lu)\n", lp);
	switch (reinterpret_cast<NMHDR *>(lp)->code)
	{
	case TCN_KEYDOWN:
	{
		auto kd = reinterpret_cast<NMTCKEYDOWN *>(lp);
		::SendMessageW(this->getHandle(), MainWindow::WM_SPECIALKEYDOWN, kd->wVKey, kd->flags);
		break;
	}
	case TCN_SELCHANGING:
		return FALSE;
	case TCN_SELCHANGE:
		this->m_tabs->selChange();
		break;
	}

	return 0;
}
void pdfv::MainWindow::wOnMove(LPARAM lp) noexcept
{
	DEBUGPRINT("pdfv::MainWindow::wOnMove(%lu)\n", lp);
	this->m_pos = { LOWORD(lp), HIWORD(lp) };
}
void pdfv::MainWindow::wOnSizing(WPARAM wp, LPARAM lp) noexcept
{
	DEBUGPRINT("pdfv::MainWindow::wOnSizing(%u, %lu)\n", wp, lp);
	auto r{ reinterpret_cast<RECT *>(lp) };
	if ((r->right - r->left) < this->m_minArea.x)
	{
		switch (wp)
		{
		case WMSZ_BOTTOMRIGHT:
		case WMSZ_RIGHT:
		case WMSZ_TOPRIGHT:
			r->right = r->left + this->m_minArea.x;
			break;
		case WMSZ_BOTTOMLEFT:
		case WMSZ_LEFT:
		case WMSZ_TOPLEFT:
			r->left = r->right - this->m_minArea.x;
			break;
		}
	}
	if ((r->bottom - r->top) < this->m_minArea.y)
	{
		switch (wp)
		{
		case WMSZ_BOTTOM:
		case WMSZ_BOTTOMLEFT:
		case WMSZ_BOTTOMRIGHT:
			r->bottom = r->top + this->m_minArea.y;
			break;
		case WMSZ_TOP:
		case WMSZ_TOPLEFT:
		case WMSZ_TOPRIGHT:
			r->top = r->bottom - this->m_minArea.y;
			break;
		}
	}
}
void pdfv::MainWindow::wOnSize() noexcept
{
	DEBUGPRINT("pdfv::MainWindow::wOnSize()\n");
	this->m_usableArea = w::getCliR(this->getHandle());


	this->m_totalArea = w::getWinR(this->getHandle());
	this->m_border = this->m_totalArea - this->m_usableArea;
	this->m_border.y -= this->m_menuSize;

	auto statusr{ w::getCliR(this->m_statushwnd) };
	this->m_tabs->resize(this->m_usableArea - xy<int>{ 0, statusr.bottom - statusr.top });
	w::resize(this->m_statushwnd, 0, 0);
	
	// Set status bar parts
	this->setStatusParts();
}
void pdfv::MainWindow::wOnCreate(HWND hwnd, LPARAM lp) noexcept
{
	DEBUGPRINT("pdfv::MainWindow::wOnCreate(%p, %lu)\n", static_cast<void *>(hwnd), lp);
	this->m_hwnd = hwnd;
	
	this->m_statushwnd = ::CreateWindowExW(
		0,
		STATUSCLASSNAMEW,
		nullptr,
		SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0,
		hwnd,
		reinterpret_cast<HMENU>(IDC_STATUSBAR),
		this->m_hInst,
		nullptr
	);
	
	auto winr{ w::getWinR(hwnd) };
	auto clir{ w::getCliR(hwnd) };
	this->m_border = make_xy(winr) - make_xy(clir);
	this->m_minArea = this->m_usableArea + this->m_border + xy<int>{ 0, this->m_menuSize };
	this->m_pos = { winr.left, winr.top };

	this->m_totalArea   = winr;
	this->m_totalArea.x = std::max(this->m_totalArea.x, this->m_minArea.x);
	this->m_totalArea.y = std::max(this->m_totalArea.y, this->m_minArea.y);
	w::resize(hwnd, this->m_totalArea.x, this->m_totalArea.y, true);

	this->m_title = w::getWinText(hwnd, MainWindow::defaulttitle);

	clir = w::getCliR(hwnd);
	auto statusr{ w::getCliR(this->m_statushwnd) };

	this->setStatusParts();

	clir.bottom = clir.top + (clir.bottom - clir.top) - (statusr.bottom - statusr.top);

	this->m_tabs = std::make_unique<Tabs>(*this, hwnd, this->getHinst(), clir);
	w::setFont(this->m_tabs->getTabsHandle(), this->getDefFont());

	this->m_tabs->insert(Tabs::defaulttitle);

	// Open PDF if any
	const wchar_t * fname{ static_cast<wchar_t *>(reinterpret_cast<CREATESTRUCTW *>(lp)->lpCreateParams) };
	if ((fname != nullptr) && (fname[0] != '\0'))
	{
		// Open pdf
		std::wstring_view fnv(fname);
		this->openPdfFile(fnv);
		this->m_openDialog.updateName(fnv);
	}

	// Create thread to check for highlighting
	this->m_moveThread = ::CreateThread(
		nullptr,
		20 * sizeof(std::size_t),
		[](LPVOID lpParam) -> DWORD WINAPI
		{
			auto self = static_cast<MainWindow *>(lpParam);

			while (!self->m_moveKillSwitch)
			{
				if (self->m_highlighted)
				{
					if (self->intersectsTabClose() == false && self->m_highlighted)
					{
						self->m_highlighted = false;
						self->m_tabs->redrawTabs();
					}
				}
				else if (self->m_highlighted == false && self->m_closeButtonDown)
				{
					self->m_closeButtonDown = false;
					self->m_tabs->redrawTabs();
				}

				::Sleep(16);
			}

			return 0;
		},
		this,
		0,
		nullptr
	);

}
void pdfv::MainWindow::wOnCopydata(LPARAM lp) noexcept
{
	DEBUGPRINT("pdfv::MainWindow::wOnCopyData(%lu)\n", lp);
	if (auto receive{ reinterpret_cast<const COPYDATASTRUCT *>(lp) }; receive != nullptr && receive->dwData > 0) [[likely]]
	{
		// Open pdf
		std::wstring_view fnv(static_cast<wchar_t *>(receive->lpData));
		if (fnv.length() == 0) [[unlikely]]
		{
			return;
		}
		this->openPdfFile(fnv);
		this->m_openDialog.updateName(fnv);
	}
}
void pdfv::MainWindow::wOnBringToFront() noexcept
{
	DEBUGPRINT("pdfv::MainWindow::wOnBringToFront()\n");
	::ShowWindow(this->getHandle(), SW_RESTORE);
	HWND topWnd{ ::GetForegroundWindow() };

	DWORD dwMyID { ::GetWindowThreadProcessId(this->getHandle(), nullptr) };
	DWORD dwCurID{ ::GetWindowThreadProcessId(topWnd, nullptr) };
	::AttachThreadInput(dwCurID, dwMyID, TRUE);
	::SetWindowPos(this->getHandle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	::SetWindowPos(this->getHandle(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
	::SetForegroundWindow(this->getHandle());
	::SetFocus(this->getHandle());
	::SetActiveWindow(this->getHandle());
	::AttachThreadInput(dwCurID, dwMyID, FALSE);
}


INT_PTR CALLBACK pdfv::MainWindow::aboutProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp) noexcept
{
	static MainWindow * self{ nullptr };

	switch (uMsg)
	{
	case WM_COMMAND:
		switch (wp)
		{
		case IDB_GITHUB:
			w::openWeb(PRODUCT_GITHUB_PAGE);
			return TRUE;
		case IDOK:
		case IDCANCEL:
			::EndDialog(hwnd, wp);
			self = nullptr;
			return TRUE;
		}
		break;
	case WM_INITDIALOG:
		self = reinterpret_cast<MainWindow *>(lp);

		// Re-position about box
		auto r1{ w::getWinR(hwnd) };
		auto r2{ w::getWinR(self->getHandle()) };

		// Move dialog to the center of parent window
		w::moveWin(
			hwnd,
			r2.left + ((r2.right  - r2.left) - (r1.right  - r1.left)) / 2,
			r2.top  + ((r2.bottom - r2.top)  - (r1.bottom - r1.top)) / 2
		);

		::SetDlgItemTextW(hwnd, IDC_ABOUT_STATIC, self->m_aboutText.c_str());

		return TRUE;
	}

	return FALSE;
}

void pdfv::MainWindow::openPdfFile(std::wstring_view file) noexcept
{
	DEBUGPRINT("pdfv::MainWindow::openPdfFile(%p)\n", static_cast<const void *>(file.data()));
	std::wstring_view fshort;
	for (std::size_t i = file.length() - 1; i > 0; --i)
	{
		if (file[i] == L'\\') [[unlikely]]
		{
			fshort = file.substr(i + 1);
			break;
		}
	}
	pdfv::Tabs::ListType::iterator it;
	if (this->m_tabs->size() == 1 && this->m_tabs->getName() == Tabs::defaulttitlepadded)
	{
		it = this->m_tabs->rename(fshort);
	}
	else
	{
		it = this->m_tabs->insert(fshort);
	}
	
	(it)->second.pdfLoad(*this, std::wstring(file));
	
	this->m_tabs->select();
}
