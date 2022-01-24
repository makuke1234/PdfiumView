#include "tabs.hpp"
#include "mainwindow.hpp"

#include <unordered_map>
#include <algorithm>

pdfv::TabObject::TabObject(std::wstring_view v1, pdfv::Pdfium && v2)
	: first(std::wstring(v1) + pdfv::Tabs::padding), second(std::move(v2))
{
}
pdfv::TabObject::TabObject(std::wstring && v1, pdfv::Pdfium && v2)
	: first(std::move(v1)), second(std::move(v2))
{
	this->first.append(pdfv::Tabs::padding);
}



[[nodiscard]] pdfv::TabObject * pdfv::Tabs::curTab() noexcept
{
	if (this->m_tabs.empty())
	{
		return nullptr;
	}
	return &this->m_tabs[this->m_tabindex];
}
[[nodiscard]] const pdfv::TabObject * pdfv::Tabs::curTab() const noexcept
{
	if (this->m_tabs.empty())
	{
		return nullptr;
	}
	return &this->m_tabs[this->m_tabindex];
}

LRESULT pdfv::Tabs::tabsCanvasProc(UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		auto hdc{ ::BeginPaint(this->m_canvashwnd, &ps) };

		auto tab{ this->curTab() };
		auto tabsize{ this->m_size - this->m_offset };
		
		// Double-buffering start
		auto memdc { ::CreateCompatibleDC(hdc) };
		auto hbmbuf{ ::CreateCompatibleBitmap(hdc, tabsize.x, tabsize.y) };
		auto hbmold{ ::SelectObject(memdc, hbmbuf) };

		// Draw here

		// Draw background
		RECT r{ .left = 0, .top = 0, .right = tabsize.x, .bottom = tabsize.y };
		::FillRect(memdc, &r, reinterpret_cast<HBRUSH>(COLOR_WINDOW));

		if (tab != nullptr && tab->second.pdfExists())
		{
			tab->second.pageRender(memdc, { 0, 0 }, tabsize);
		}
		
		// Double-buffering end
		::BitBlt(hdc, 0, 0, tabsize.x, tabsize.y, memdc, 0, 0, SRCCOPY);
		
		::SelectObject(memdc, hbmold);

		::DeleteObject(hbmbuf);
		::DeleteDC(memdc);

		::EndPaint(this->m_canvashwnd, &ps);
		break;
	}
	case WM_ERASEBKGND:
		return TRUE;
	case WM_MOUSEMOVE:
		::SendMessageW(this->m_tabshwnd, MainWindow::WM_TABMOUSEMOVE, wp, lp);
		break;
	case WM_VSCROLL:
	{
		auto tab{ this->curTab() };
		if (tab != nullptr && tab->second.pdfExists())
		{
			int yNewPos;

			switch (LOWORD(wp))
			{
			case SB_PAGEUP:
			case SB_LINEUP:
				yNewPos = tab->page - 1;
				break;
			case SB_PAGEDOWN:
			case SB_LINEDOWN:
				yNewPos = tab->page + 1;
				break;
			case SB_TOP:
				yNewPos = 0;
				break;
			case SB_BOTTOM:
				yNewPos = tab->yMaxScroll;
				break;
			case SB_THUMBPOSITION:
				yNewPos = HIWORD(wp);
				break;
			case SB_THUMBTRACK:
			{
				SCROLLINFO si{};
				si.cbSize = sizeof si;
				si.fMask  = SIF_TRACKPOS;
				::GetScrollInfo(this->m_canvashwnd, SB_VERT, &si);

				yNewPos = si.nTrackPos;
				break;
			}
			default:
				yNewPos = tab->page;
			}
			yNewPos = std::max(yNewPos, 0);
			yNewPos = std::min(yNewPos, tab->yMaxScroll);

			if (yNewPos == tab->page)
			{
				break;
			}

			tab->page = yNewPos;

			tab->second.pageLoad(tab->page + 1);
			this->updatePageCounter();
			w::redraw(this->m_canvashwnd);

			SCROLLINFO si{};
			si.cbSize = sizeof si;
			si.fMask  = SIF_POS;
			si.nPos   = tab->page;
			::SetScrollInfo(this->m_canvashwnd, SB_VERT, &si, true);
		}
		break;
	}
	case Tabs::WM_ZOOM:
	{
		auto tab{ this->curTab() };
		if (tab != nullptr && tab->second.pdfExists())
		{
			auto prevzoom{ tab->zoom };
			tab->zoom += float(int(wp)) / float(WHEEL_DELTA);
			tab->zoom = std::clamp(tab->zoom, 1.0f, 10.0f);
			if (prevzoom != tab->zoom)
			{
				this->updateZoom();
			}	
		}
		break;
	}
	case Tabs::WM_ZOOMRESET:
	{
		auto tab{ this->curTab() };
		if (tab != nullptr && tab->second.pdfExists() && tab->zoom != 1.0f)
		{
			tab->zoom = 1.0f;
			this->updateZoom();
		}
		break;
	}
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		::SendMessageW(this->m_tabshwnd, msg, wp, lp);
		break;
	case WM_CREATE:
		this->updateScrollbar();
		this->updatePageCounter();
		this->updateZoom();
		break;
	default:
		return ::DefWindowProcW(this->m_canvashwnd, msg, wp, lp);
	}

	return 0;
}

void pdfv::Tabs::updatePageCounter() const noexcept
{
	if (auto tab{ this->curTab() }; tab != nullptr && tab->second.pdfExists())
	{
		setText(
			window.getStatusHandle(), MainWindow::StatusPages, w::status::DrawOp::def,
			(std::wstring(L"Page: ") + std::to_wstring(tab->second.pageGetNum()) + L'/' + std::to_wstring(tab->second.pageGetCount())).c_str()
		);
	}
	else
	{
		setText(window.getStatusHandle(), MainWindow::StatusPages, w::status::DrawOp::def, L"Page: NA");
	}
	
}
void pdfv::Tabs::updateZoom() const noexcept
{
	if (auto tab{ this->curTab() }; tab != nullptr && tab->second.pdfExists())
	{
		setText(
			window.getStatusHandle(), MainWindow::StatusZoom, w::status::DrawOp::def,
			(std::wstring(L"Zoom: ") + std::to_wstring(uint32_t(tab->zoom * 100.0f + 0.5f)) + L'%').c_str()
		);
	}
	else
	{
		setText(window.getStatusHandle(), MainWindow::StatusZoom, w::status::DrawOp::def, L"Zoom: NA");
	}
}

pdfv::Tabs::Tabs(HWND hwnd, HINSTANCE hInst, RECT size) noexcept
	: m_parent(hwnd), m_pos(size.left, size.top), m_size(size)
{
	DEBUGPRINT("pdfv::Tabs::Tabs(%p, %p)\n", static_cast<void *>(hwnd), static_cast<void *>(hInst));
	
	this->m_tabshwnd = ::CreateWindowExW(
		0,
		WC_TABCONTROL,
		nullptr,
		WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED,
		this->m_pos.x, this->m_pos.y,
		this->m_size.x, this->m_size.y,
		this->m_parent,
		nullptr,
		hInst,
		nullptr
	);

	::SetWindowSubclass(
		this->m_tabshwnd,
		[](HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp, UINT_PTR, DWORD_PTR) -> LRESULT CALLBACK
		{
			if (umsg == WM_MOUSEMOVE || umsg == MainWindow::WM_TABMOUSEMOVE)
			{
				::SendMessageW(::GetParent(hwnd), MainWindow::WM_TABMOUSEMOVE, wp, lp);
				return TRUE;
			}
			else if (umsg == WM_LBUTTONDOWN || umsg == WM_LBUTTONUP)
			{
				::SendMessageW(::GetParent(hwnd), umsg, wp, lp);
				// Block making the tab active if the tab close button is being pressed
				if (window.m_highlighted)
				{
					return TRUE;
				}
			}
			else if (umsg == WM_KEYUP)
			{
				::SendMessageW(::GetParent(hwnd), umsg, wp, lp);
			}

			return ::DefSubclassProc(hwnd, umsg, wp, lp);
		},
		1,
		0
	);

	this->m_canvashwnd = ::CreateWindowExW(
		0,
		c_className,
		nullptr,
		WS_CHILD | WS_TABSTOP | WS_CLIPSIBLINGS | WS_VSCROLL | WS_VISIBLE,
		this->m_offset.x, this->m_offset.y,
		this->m_size.x - this->m_offset.x, this->m_size.y - this->m_offset.y,
		this->m_tabshwnd,
		nullptr,
		hInst,
		this
	);
}
pdfv::Tabs::Tabs(pdfv::Tabs && other) noexcept
	: m_parent(other.m_parent), m_tabshwnd(other.m_tabshwnd),
	m_canvashwnd(other.m_canvashwnd), m_pos(std::move(other.m_pos)),
	m_size(std::move(other.m_size)), m_offset(std::move(other.m_offset)),
	m_tabs(std::move(other.m_tabs)), m_tabindex(other.m_tabindex)
{
	other.m_parent     = nullptr;
	other.m_tabshwnd   = nullptr;
	other.m_canvashwnd = nullptr;
	other.m_tabindex   = 0;
}
pdfv::Tabs & pdfv::Tabs::operator=(pdfv::Tabs && other) noexcept
{
	this->m_parent     = other.m_parent;
	this->m_tabshwnd   = other.m_tabshwnd;
	this->m_canvashwnd = other.m_canvashwnd;
	this->m_pos        = std::move(other.m_pos);
	this->m_size       = std::move(other.m_size);
	this->m_offset     = std::move(other.m_offset);
	this->m_tabs       = std::move(other.m_tabs);
	this->m_tabindex   = other.m_tabindex;

	other.m_parent     = nullptr;
	other.m_tabshwnd   = nullptr;
	other.m_canvashwnd = nullptr;
	other.m_tabindex   = 0;

	return *this;
}
pdfv::Tabs::~Tabs() noexcept
{
	this->m_tabs.clear();
	if (this->m_canvashwnd != nullptr) [[likely]]
	{
		::DestroyWindow(this->m_canvashwnd);
		this->m_canvashwnd = nullptr;
	}
	if (this->m_tabshwnd != nullptr) [[likely]]
	{
		::DestroyWindow(this->m_tabshwnd);
		this->m_tabshwnd = nullptr;
	}
	this->m_parent = nullptr;
}

LRESULT CALLBACK pdfv::Tabs::tabsCanvasProcHub(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
	Tabs * self{ nullptr };
	if (uMsg == WM_CREATE) [[unlikely]]
	{
		auto cs{ reinterpret_cast<CREATESTRUCTW *>(lp) };
		self = static_cast<Tabs *>(cs->lpCreateParams);
		self->m_canvashwnd = hwnd;
		::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
	}
	else [[likely]]
	{
		self = reinterpret_cast<Tabs *>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
	}

	if (self != nullptr) [[likely]]
	{
		return self->tabsCanvasProc(uMsg, wp, lp);
	}
	else [[unlikely]]
	{
		return ::DefWindowProcW(hwnd, uMsg, wp, lp);
	}
}

void pdfv::Tabs::resize(xy<int> newsize) noexcept
{
	if (newsize != this->m_size)
	{
		this->m_size = newsize;
		w::resize(this->getTabsHandle(), this->m_size.x, this->m_size.y);
		w::resize(this->getCanvasHandle(), this->m_size.x - this->m_offset.x, this->m_size.y - this->m_offset.y);
	}
}
void pdfv::Tabs::move(xy<int> newpos) noexcept
{
	if (newpos != this->m_pos)
	{
		this->m_pos = newpos;
		w::moveWin(this->getTabsHandle(), this->m_pos.x, this->m_pos.y);
	}
}
void pdfv::Tabs::redrawTabs() noexcept
{
	w::redraw(this->getTabsHandle());
	this->updateScrollbar();
	this->updatePageCounter();
	this->updateZoom();
}
void pdfv::Tabs::redrawCanvas() const noexcept
{
	w::redraw(this->getCanvasHandle());
	this->updatePageCounter();
	this->updateZoom();
}

[[nodiscard]] RECT pdfv::Tabs::s_calcCloseButton(RECT itemRect) noexcept
{
	RECT r;
	r.left   = itemRect.right - dip(s_cCloseButtonSz.x + 2, dpi.x);
	r.top    = (itemRect.top + itemRect.bottom) / 2 - dip(s_cCloseButtonSz.y, dpi.y) / 2;
	r.right  = r.left + dip(s_cCloseButtonSz.x, dpi.x);
	r.bottom = r.top  + dip(s_cCloseButtonSz.y, dpi.y);
	return r;
}

pdfv::Tabs::ListType::iterator pdfv::Tabs::insert(std::wstring_view title, const pdfv::ssize_t index)
{
	TCITEMW tie{};
	tie.mask   = TCIF_TEXT | TCIF_IMAGE;
	tie.iImage = -1;

	pdfv::Tabs::ListType::iterator ret;
	if (index == Tabs::endpos)
	{
		// Insert at the end
		this->m_tabs.emplace_back(title);
		ret = this->m_tabs.end();
		--ret;

		tie.pszText = (ret)->first.data();
		TabCtrl_InsertItem(this->m_tabshwnd, this->m_tabs.size() - 1, &tie);
	}
	else
	{
		ret = this->m_tabs.insert(
			this->m_tabs.begin() + index,
			TabObject(title)
		);
		tie.pszText = (ret)->first.data();
		TabCtrl_InsertItem(this->m_tabshwnd, index, &tie);
	}
	if (!this->m_offset.y)
	{
		this->m_offset.y = this->m_pos.y + dip(23, dpi.y);
	}

	return ret;
}
void pdfv::Tabs::remove(std::wstring_view title) noexcept
{
	std::size_t index{ 0 };
	for (auto it{ this->m_tabs.begin() }; it != this->m_tabs.end(); ++it, ++index)
	{
		if ((it)->first == title)
		{
			this->m_tabs.erase(it);
			TabCtrl_DeleteItem(this->m_tabshwnd, index);
			break;
		}
	}
}
void pdfv::Tabs::remove(pdfv::ssize_t index) noexcept
{
	printf("Index: %d\n", index);
	if (index == Tabs::endpos)
	{
		// Remove from the end
		this->m_tabs.pop_back();
		TabCtrl_DeleteItem(this->m_tabshwnd, this->m_tabs.size());
	}
	else
	{
		index = std::min(index, ssize_t(this->m_tabs.size()));
		auto it{ this->m_tabs.begin() + index };
		this->m_tabs.erase(it);
		TabCtrl_DeleteItem(this->m_tabshwnd, index);
	}
}
pdfv::Tabs::ListType::iterator pdfv::Tabs::rename(std::wstring_view title, const pdfv::ssize_t index)
{
	assert(!this->m_tabs.empty());

	TCITEMW tie{};
	tie.mask   = TCIF_TEXT | TCIF_IMAGE;
	tie.iImage = -1;

	if (index == Tabs::endpos)
	{
		// Rename the end
		auto it{ this->m_tabs.end() };
		--it;

		(it)->first.assign(title);
		(it)->first += pdfv::Tabs::padding;
		tie.pszText = (it)->first.data();
		TabCtrl_SetItem(this->m_tabshwnd, this->m_tabs.size() - 1, &tie);
		return it;
	}
	else
	{
		auto it{ this->m_tabs.begin() + index };

		(it)->first.assign(title);
		(it)->first += pdfv::Tabs::padding;
		tie.pszText = (it)->first.data();
		TabCtrl_SetItem(this->m_tabshwnd, index, &tie);
		return it;
	}
}
[[nodiscard]] std::wstring_view pdfv::Tabs::getName(const pdfv::ssize_t index) const noexcept
{
	if (index == Tabs::endpos)
	{
		return this->m_tabs.back().first;
	}
	else
	{
		return this->m_tabs[index].first;
	}
}
void pdfv::Tabs::select(const ssize_t index) noexcept
{
	if (index == Tabs::endpos)
	{
		TabCtrl_SetCurSel(this->m_tabshwnd, this->m_tabs.size() - 1);
	}
	else
	{
		TabCtrl_SetCurSel(this->m_tabshwnd, std::clamp(index, ssize_t(0), ssize_t(this->m_tabs.size() - 1)));
	}
	this->selChange();
}
[[nodiscard]] std::size_t pdfv::Tabs::size() const noexcept
{
	return this->m_tabs.size();
}
void pdfv::Tabs::selChange() noexcept
{
	this->m_tabindex = TabCtrl_GetCurSel(this->m_tabshwnd);

	this->redrawTabs();
}

void pdfv::Tabs::updateScrollbar() noexcept
{
	if (auto tab{ this->curTab() }; tab != nullptr && tab->second.pdfExists())
	{
		::ShowScrollBar(this->m_canvashwnd, SB_VERT, TRUE);

		tab->yMaxScroll = tab->second.pageGetCount() - 1;
		tab->page = std::min(int(tab->second.pageGetNum() - 1), tab->yMaxScroll);

		SCROLLINFO si{};
		si.cbSize = sizeof si;
		si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
		si.nMin   = tab->yMinScroll;
		si.nMax   = tab->yMaxScroll;
		si.nPage  = 1;
		si.nPos   = tab->page;
		::SetScrollInfo(this->m_canvashwnd, SB_VERT, &si, TRUE);
	}
	else
	{
		::ShowScrollBar(this->m_canvashwnd, SB_VERT, FALSE);
	}
}

