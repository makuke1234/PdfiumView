#include "tabs.hpp"
#include "mainwindow.hpp"

#include <unordered_map>
#include <algorithm>

namespace pdfv
{
	[[nodiscard]] static HWND createTabobjectHWND(const HWND hwnd, const HINSTANCE hInst, RECT r, TabObject * self) noexcept
	{
		DEBUGPRINT("pdfv::createTabobjectHWND(%p, %p)\n", static_cast<const void *>(hwnd), static_cast<const void *>(hInst));
		return ::CreateWindowExW(
			0,
			APP_CLASSNAME "Tab",
			nullptr,
			WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP,
			r.left,
			r.top,
			r.right  - r.left,
			r.bottom - r.top,
			hwnd,
			nullptr,
			hInst,
			self
		);
	}
}

pdfv::TabObject::TabObject(HWND tabshwnd, HINSTANCE hInst, RECT r) noexcept
	: parent(tabshwnd), tabhandle(pdfv::createTabobjectHWND(parent, hInst, r, this))
{
	DEBUGPRINT("pdfv::TabObject::TabObject(%p, %p)\n", static_cast<void *>(tabshwnd), static_cast<void *>(hInst));
}
pdfv::TabObject::TabObject(HWND tabshwnd, HINSTANCE hInst, RECT r, std::wstring_view V1, pdfv::Pdfium && V2)
	: first(std::wstring(V1) + pdfv::Tabs::padding), second(std::move(V2)),
	parent(tabshwnd), tabhandle(pdfv::createTabobjectHWND(tabshwnd, hInst, r, this))
{
	DEBUGPRINT("pdfv::TabObject::TabObject(%p, %p, %p, %p)\n", static_cast<void *>(tabshwnd), static_cast<void *>(hInst), static_cast<const void *>(V1.data()), static_cast<void *>(&V2));
}
pdfv::TabObject::TabObject(HWND tabshwnd, HINSTANCE hInst, RECT r, std::wstring && V1, pdfv::Pdfium && V2) noexcept
	: first(std::move(V1)), second(std::move(V2)),
	parent(tabshwnd), tabhandle(pdfv::createTabobjectHWND(tabshwnd, hInst, r, this))
{
	DEBUGPRINT("pdfv::TabObject::TabObject(%p, %p, %p, %p)\n", static_cast<void *>(tabshwnd), static_cast<void *>(hInst), static_cast<const void *>(V1.c_str()), static_cast<void *>(&V2));
	this->first += pdfv::Tabs::padding;
}
pdfv::TabObject::TabObject(TabObject && other) noexcept
	: first(std::move(other.first)), second(std::move(other.second)),
	parent(other.parent), tabhandle(other.tabhandle),
	size(std::move(other.size)), yMaxScroll(other.yMaxScroll), yMinScroll(other.yMinScroll),
	page(other.page)
{
	DEBUGPRINT("pdfv::TabObject(TabObject && %p)\n", static_cast<void *>(&other));
	other.parent    = nullptr;
	other.tabhandle = nullptr;
}
pdfv::TabObject & pdfv::TabObject::operator=(TabObject && other) noexcept
{
	DEBUGPRINT("pdfv::TabObject::operator=(%p)\n", static_cast<void *>(&other));
	this->~TabObject();

	this->first      = std::move(other.first);
	this->second     = std::move(other.second);
	this->parent     = other.parent;
	this->tabhandle  = other.tabhandle;
	this->size       = std::move(other.size);
	this->yMaxScroll = other.yMaxScroll;
	this->yMinScroll = other.yMinScroll;
	this->page       = other.page;

	other.parent     = nullptr;
	other.tabhandle  = nullptr;

	return *this;
}
pdfv::TabObject::~TabObject() noexcept
{
	DEBUGPRINT("pdfv::TabObject::~TabObject()\n");
	if (this->tabhandle != nullptr) [[likely]]
	{
		::DestroyWindow(this->tabhandle);
		this->tabhandle = nullptr;
	}
	this->parent = nullptr;
}

void pdfv::TabObject::show(bool visible) const noexcept
{
	DEBUGPRINT("pdfv::TabObject::show()\n");
	::ShowWindow(this->tabhandle, visible ? SW_SHOW : SW_HIDE);
}
void pdfv::TabObject::updatePDF() noexcept
{
	DEBUGPRINT("pdfv::TabObject::updatePDF()\n");
	this->updateScrollbar();
	this->updatePageCounter();
	this->updateZoom();
}

LRESULT CALLBACK pdfv::TabObject::tabObjectProcHub(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
	TabObject * self{ nullptr };
	if (uMsg == WM_CREATE) [[unlikely]]
	{
		auto cs{ reinterpret_cast<CREATESTRUCTW *>(lp) };
		self = static_cast<TabObject *>(cs->lpCreateParams);
		self->tabhandle = hwnd;
		::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
	}
	else [[likely]]
	{
		self = reinterpret_cast<TabObject *>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
	}

	if (self != nullptr) [[likely]]
	{
		return self->tabObjectProc(uMsg, wp, lp);
	}
	else [[unlikely]]
	{
		return ::DefWindowProcW(hwnd, uMsg, wp, lp);
	}
}

LRESULT pdfv::TabObject::tabObjectProc(UINT uMsg, WPARAM wp, LPARAM lp)
{
	switch (uMsg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		auto hdc{ ::BeginPaint(this->tabhandle, &ps) };
	
		// Double-buffering start
		auto memdc { ::CreateCompatibleDC(hdc) };
		auto hbmbuf{ ::CreateCompatibleBitmap(hdc, this->size.x, this->size.y) };
		auto hbmold{ ::SelectObject(memdc, hbmbuf) };

		// Draw here

		// Draw background
		RECT r{ .left = 0, .top = 0, .right = this->size.x, .bottom = this->size.y };
		::FillRect(memdc, &r, reinterpret_cast<HBRUSH>(COLOR_WINDOW));

		if (this->second.pdfExists())
		{
			this->second.pageRender(memdc, { 0, 0 }, this->size);
		}
		
		// Double-buffering end
		::BitBlt(hdc, 0, 0, this->size.x, this->size.y, memdc, 0, 0, SRCCOPY);
		
		::SelectObject(memdc, hbmold);

		::DeleteObject(hbmbuf);
		::DeleteDC(memdc);

		::EndPaint(this->tabhandle, &ps);
		break;
	}
	case WM_ERASEBKGND:
		return TRUE;
	case WM_MOUSEMOVE:
		::SendMessageW(::GetParent(this->tabhandle), MainWindow::WM_TABMOUSEMOVE, wp, lp);
		break;
	case WM_VSCROLL:
	{
		if (this->second.pdfExists())
		{
			int yNewPos;

			switch (LOWORD(wp))
			{
			case SB_PAGEUP:
			case SB_LINEUP:
				yNewPos = this->page - 1;
				break;
			case SB_PAGEDOWN:
			case SB_LINEDOWN:
				yNewPos = this->page + 1;
				break;
			case SB_TOP:
				yNewPos = 0;
				break;
			case SB_BOTTOM:
				yNewPos = this->yMaxScroll;
				break;
			case SB_THUMBPOSITION:
				yNewPos = HIWORD(wp);
				break;
			case SB_THUMBTRACK:
			{
				SCROLLINFO si{};
				si.cbSize = sizeof si;
				si.fMask  = SIF_TRACKPOS;
				::GetScrollInfo(this->tabhandle, SB_VERT, &si);

				yNewPos = si.nTrackPos;
				break;
			}
			default:
				yNewPos = this->page;
			}
			yNewPos = std::max(yNewPos, 0);
			yNewPos = std::min(yNewPos, this->yMaxScroll);

			if (yNewPos == this->page)
			{
				break;
			}

			this->page = yNewPos;

			this->second.pageLoad(this->page + 1);
			this->updatePageCounter();
			w::redraw(this->tabhandle);

			SCROLLINFO si{};
			si.cbSize = sizeof si;
			si.fMask  = SIF_POS;
			si.nPos   = this->page;
			::SetScrollInfo(this->tabhandle, SB_VERT, &si, true);
		}
		break;
	}
	case TabObject::WM_ZOOM:
		if (this->second.pdfExists())
		{
			auto prevzoom{ this->zoom };
			this->zoom += float(int(wp)) / float(WHEEL_DELTA);
			this->zoom = std::clamp(this->zoom, 1.0f, 10.0f);
			if (prevzoom != this->zoom)
			{
				this->updateZoom();
			}	
		}
		break;
	case TabObject::WM_ZOOMRESET:
		if (this->second.pdfExists() && this->zoom != 1.0f)
		{
			this->zoom = 1.0f;
			this->updateZoom();
		}
		break;
	case WM_COMMAND:
		::SendMessageW(this->parent, WM_COMMAND, wp, lp);
		break;
	case WM_SIZE:
		this->size = w::getCliR(this->tabhandle);
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		::SendMessageW(::GetParent(this->tabhandle), uMsg, wp, lp);
		break;
	case WM_CLOSE:
		::DestroyWindow(this->tabhandle);
		this->tabhandle = nullptr;
		break;
	case WM_CREATE:
		this->updatePageCounter();
		this->updateZoom();
		break;
	default:
		return ::DefWindowProcW(this->tabhandle, uMsg, wp, lp);
	}

	return 0;
}

void pdfv::TabObject::updateScrollbar() noexcept
{
	if (this->second.pdfExists())
	{
		this->yMaxScroll = this->second.pageGetCount() - 1;
		this->page = std::min(int(this->second.pageGetNum() - 1), this->yMaxScroll);

		SCROLLINFO si{};
		si.cbSize = sizeof si;
		si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
		si.nMin   = this->yMinScroll;
		si.nMax   = this->yMaxScroll;
		si.nPage  = 1;
		si.nPos   = this->page;
		::SetScrollInfo(this->tabhandle, SB_VERT, &si, TRUE);
	}
}
void pdfv::TabObject::updatePageCounter() const noexcept
{
	if (this->second.pdfExists())
	{
		setText(
			window.getStatusHandle(), MainWindow::StatusPages, w::status::DrawOp::def,
			(std::wstring(L"Page: ") + std::to_wstring(this->second.pageGetNum()) + L'/' + std::to_wstring(this->second.pageGetCount())).c_str()
		);
	}
	else
	{
		setText(window.getStatusHandle(), MainWindow::StatusPages, w::status::DrawOp::def, L"Page: NA");
	}
}
void pdfv::TabObject::updateZoom() const noexcept
{
	if (this->second.pdfExists())
	{
		setText(
			window.getStatusHandle(), MainWindow::StatusZoom, w::status::DrawOp::def,
			(std::wstring(L"Zoom: ") + std::to_wstring(uint32_t(this->zoom * 100.0f + 0.5f)) + L'%').c_str()
		);
	}
	else
	{
		setText(window.getStatusHandle(), MainWindow::StatusZoom, w::status::DrawOp::def, L"Zoom: NA");
	}
}

pdfv::Tabs::Tabs(HWND hwnd, HINSTANCE hInst, RECT size) noexcept
	: m_parent(hwnd)
{
	DEBUGPRINT("pdfv::Tabs::Tabs(%p, %p)\n", static_cast<void *>(hwnd), static_cast<void *>(hInst));
	this->m_tabshwnd = ::CreateWindowExW(
		0,
		WC_TABCONTROL,
		nullptr,
		WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED,
		0, 0,
		size.right - size.left, size.bottom - size.top,
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

			return ::DefSubclassProc(hwnd, umsg, wp, lp);
		},
		1,
		0
	);
}
pdfv::Tabs::Tabs(pdfv::Tabs && other) noexcept
	: m_parent(other.m_parent), m_tabshwnd(other.m_tabshwnd), m_pos(std::move(other.m_pos)),
	m_size(std::move(other.m_size)), m_offset(std::move(other.m_offset)),
	m_tabs(std::move(other.m_tabs)), m_tabindex(other.m_tabindex)
{
	other.m_parent   = nullptr;
	other.m_tabshwnd = nullptr;
	other.m_tabindex = 0;
}
pdfv::Tabs & pdfv::Tabs::operator=(pdfv::Tabs && other) noexcept
{
	this->m_parent   = other.m_parent;
	this->m_tabshwnd = other.m_tabshwnd;
	this->m_pos      = std::move(other.m_pos);
	this->m_size     = std::move(other.m_size);
	this->m_offset   = std::move(other.m_offset);
	this->m_tabs     = std::move(other.m_tabs);
	this->m_tabindex = other.m_tabindex;

	other.m_parent   = nullptr;
	other.m_tabshwnd = nullptr;
	other.m_tabindex = 0;

	return *this;
}
pdfv::Tabs::~Tabs() noexcept
{
	this->m_tabs.clear();
	if (this->m_tabshwnd != nullptr) [[likely]]
	{
		::DestroyWindow(this->m_tabshwnd);
		this->m_tabshwnd = nullptr;
	}
	this->m_parent = nullptr;
}

void pdfv::Tabs::resize(xy<int> newsize) noexcept
{
	std::swap(this->m_size, newsize);
	if (!this->m_tabs.empty())
	{
		if (auto r{ w::getCliR(this->m_tabs[this->m_tabindex]->tabhandle) }; (this->m_size - this->m_offset) != r)
		{
			::MoveWindow(
				this->m_tabs[this->m_tabindex]->tabhandle,
				this->m_offset.x, this->m_offset.y,
				this->m_size.x - this->m_offset.x, this->m_size.y - this->m_offset.y,
				TRUE
			);
			DEBUGPRINT("Tab resizing done!\n");
		}
	}
	if (newsize != this->m_size)
	{
		w::resize(this->getHandle(), this->m_size.x, this->m_size.y);
	}
}
void pdfv::Tabs::move(xy<int> newpos) noexcept
{
	if (newpos != this->m_pos)
	{
		this->m_pos = newpos;
		w::moveWin(this->getHandle(), this->m_pos.x, this->m_pos.y);
	}
}
void pdfv::Tabs::repaint() const noexcept
{
	w::redraw(this->getHandle());
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

	RECT TabObjR{
		.left   = this->m_offset.x,
		.top    = this->m_offset.x,
		.right  = this->m_size.x,
		.bottom = this->m_size.y
	};

	pdfv::Tabs::ListType::iterator ret;
	if (index == Tabs::endpos)
	{
		// Insert at the end
		this->m_tabs.emplace_back(std::make_unique<TabObject>(this->m_tabshwnd, window.getHinst(), TabObjR, title));
		ret = this->m_tabs.end();
		--ret;

		tie.pszText = (*ret)->first.data();
		TabCtrl_InsertItem(this->m_tabshwnd, this->m_tabs.size() - 1, &tie);
	}
	else
	{
		ret = this->m_tabs.insert(
			this->m_tabs.begin() + index,
			std::make_unique<TabObject>(this->m_tabshwnd, window.getHinst(), TabObjR, title)
		);
		tie.pszText = (*ret)->first.data();
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
		if ((*it)->first == title)
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

		(*it)->first.assign(title);
		(*it)->first += pdfv::Tabs::padding;
		tie.pszText = (*it)->first.data();
		TabCtrl_SetItem(this->m_tabshwnd, this->m_tabs.size() - 1, &tie);
		return it;
	}
	else
	{
		auto it{ this->m_tabs.begin() + index };

		(*it)->first.assign(title);
		(*it)->first += pdfv::Tabs::padding;
		tie.pszText = (*it)->first.data();
		TabCtrl_SetItem(this->m_tabshwnd, index, &tie);
		return it;
	}
}
[[nodiscard]] std::wstring_view pdfv::Tabs::getName(const pdfv::ssize_t index) const noexcept
{
	if (index == Tabs::endpos)
	{
		return this->m_tabs.back()->first;
	}
	else
	{
		return this->m_tabs[index]->first;
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
		TabCtrl_SetCurSel(this->m_tabshwnd, index);
	}
	this->selChange();
}
[[nodiscard]] std::size_t pdfv::Tabs::size() const noexcept
{
	return this->m_tabs.size();
}
void pdfv::Tabs::selChange() noexcept
{
	auto oldidx{ this->m_tabindex };
	this->m_tabindex = TabCtrl_GetCurSel(this->m_tabshwnd);

	if (this->m_tabindex == oldidx)
	{
		w::redraw(this->m_tabs[this->m_tabindex]->tabhandle);
	}
	else
	{
		this->m_tabs[oldidx]->show(false);
		this->resize(this->m_size);
	}
	this->m_tabs[this->m_tabindex]->show();
	this->m_tabs[this->m_tabindex]->updatePageCounter();
	this->m_tabs[this->m_tabindex]->updateZoom();
}
