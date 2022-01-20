#include "tabs.hpp"
#include "mainwindow.hpp"

#include <unordered_map>

namespace pdfv
{
	[[nodiscard]] static inline HWND createTabobjectHWND(const HWND hwnd, const HINSTANCE hInst, TabObject * self) noexcept
	{
		DEBUGPRINT("pdfv::createTabobjectHWND(%p, %p)\n", static_cast<const void *>(hwnd), static_cast<const void *>(hInst));
		auto r = w::getCliR(hwnd);
		return ::CreateWindowExW(
			0,
			APP_CLASSNAME "Tab",
			L"",
			WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP,
			r.left,
			r.top,
			r.right,
			r.bottom,
			hwnd,
			nullptr,
			hInst,
			self
		);
	}
}

pdfv::TabObject::TabObject(HWND tabshwnd, HINSTANCE hInst) noexcept
	: parent(tabshwnd)
{
	DEBUGPRINT("pdfv::TabObject::TabObject(%p, %p)\n", static_cast<void *>(tabshwnd), static_cast<void *>(hInst));
	this->tabhandle = pdfv::createTabobjectHWND(this->parent, hInst, this);
}
pdfv::TabObject::TabObject(HWND tabshwnd, HINSTANCE hInst, std::wstring_view V1, pdfv::Pdfium && V2)
	: first(V1), second(std::move(V2)), parent(tabshwnd)
{
	DEBUGPRINT("pdfv::TabObject::TabObject(%p, %p, %p, %p)\n", static_cast<void *>(tabshwnd), static_cast<void *>(hInst), static_cast<const void *>(V1.data()), static_cast<void *>(&V2));
	this->first    += pdfv::Tabs::padding;
	this->tabhandle = pdfv::createTabobjectHWND(this->parent, hInst, this);
}
pdfv::TabObject::TabObject(HWND tabshwnd, HINSTANCE hInst, std::wstring && V1, pdfv::Pdfium && V2) noexcept
	: first(std::move(V1)), second(std::move(V2)), parent(tabshwnd)
{
	DEBUGPRINT("pdfv::TabObject::TabObject(%p, %p, %p, %p)\n", static_cast<void *>(tabshwnd), static_cast<void *>(hInst), static_cast<const void *>(V1.c_str()), static_cast<void *>(&V2));
	this->first    += pdfv::Tabs::padding;
	this->tabhandle = pdfv::createTabobjectHWND(this->parent, hInst, this);
}
pdfv::TabObject::TabObject(TabObject && other) noexcept
	: first(std::move(other.first)), second(std::move(other.second)),
	tabhandle(other.tabhandle), parent(other.parent),
	size(std::move(other.size)), yMaxScroll(other.yMaxScroll), yMinScroll(other.yMinScroll),
	page(other.page), handles(std::move(other.handles))
{
	DEBUGPRINT("pdfv::TabObject(TabObject && %p)\n", static_cast<void *>(&other));
	other.tabhandle   = nullptr;
	other.parent      = nullptr;
}
pdfv::TabObject & pdfv::TabObject::operator=(TabObject && other) noexcept
{
	DEBUGPRINT("pdfv::TabObject::operator=(%p)\n", static_cast<void *>(&other));
	this->~TabObject();

	this->first       = std::move(other.first);
	this->second      = std::move(other.second);
	this->tabhandle   = other.tabhandle;
	this->parent      = other.parent;
	this->size        = std::move(other.size);
	this->yMaxScroll  = other.yMaxScroll;
	this->yMinScroll  = other.yMinScroll;
	this->page        = other.page;
	this->handles     = std::move(other.handles);

	other.tabhandle   = nullptr;
	other.parent      = nullptr;

	return *this;
}
pdfv::TabObject::~TabObject() noexcept
{
	DEBUGPRINT("pdfv::TabObject::~TabObject()\n");
	if (this->tabhandle != nullptr)
	{
		::DestroyWindow(this->tabhandle);
		this->tabhandle = nullptr;
	}
	this->parent = nullptr;
}

void pdfv::TabObject::insert(HWND handle)
{
	DEBUGPRINT("pdfv::TabObject::insert(%p)\n", static_cast<void *>(handle));
	auto p = ::GetParent(handle);
	if (p != this->tabhandle)
	{
		::SetParent(handle, this->tabhandle);
	}
	this->handles.insert(handle);
}
const HWND & pdfv::TabObject::insert(
	DWORD dwExStyle,
	std::wstring_view className,
	std::wstring_view windowName,
	DWORD dwStyle,
	xy<int> pos,
	xy<int> size,
	HMENU menu,
	HINSTANCE hinst
)
{
	DEBUGPRINT(
		"pdfv::TabObject::insert(%lu, %p, %p, %lu, %p, %p, %p, %p)\n",
		dwExStyle,
		static_cast<const void *>(className.data()),
		static_cast<const void *>(windowName.data()),
		dwStyle,
		static_cast<const void *>(&pos),
		static_cast<const void *>(&size),
		static_cast<void *>(menu),
		static_cast<void *>(hinst)
	);
	return *(this->handles.emplace(::CreateWindowExW(
		dwExStyle,
		className.data(),
		windowName.data(),
		dwStyle,
		pos.x,
		pos.y,
		size.x,
		size.y,
		this->tabhandle,
		menu,
		hinst,
		nullptr
	)).first);
}
void pdfv::TabObject::remove(HWND & handle) noexcept
{
	DEBUGPRINT("pdfv::TabObject::remove(%p)\n", static_cast<void *>(handle));
	::DestroyWindow(handle);
	this->handles.erase(handle);
	handle = nullptr;
}
void pdfv::TabObject::show(bool visible) const noexcept
{
	DEBUGPRINT("pdfv::TabObject::show()\n");
	::ShowWindow(this->tabhandle, visible ? SW_SHOW : SW_HIDE);
}
void pdfv::TabObject::updatePDF() const noexcept
{
	DEBUGPRINT("pdfv::TabObject::updatePDF()\n");
	auto r = w::getCliR(this->tabhandle);
	::SendMessageW(this->tabhandle, WM_SIZE, 0, MAKELONG(r.right - r.left, r.bottom - r.top));
}

LRESULT CALLBACK pdfv::TabObject::tabProcHub(const HWND hwnd, const UINT uMsg, WPARAM wp, LPARAM lp)
{
	TabObject * self;
	if (uMsg == WM_CREATE) [[unlikely]]
	{
		auto cs = reinterpret_cast<CREATESTRUCTW *>(lp);
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
		return self->tabProc(uMsg, wp, lp);
	}
	else [[unlikely]]
	{
		return ::DefWindowProcW(hwnd, uMsg, wp, lp);
	}
}

LRESULT pdfv::TabObject::tabProc(UINT uMsg, WPARAM wp, LPARAM lp)
{
	switch (uMsg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		auto hdc = ::BeginPaint(this->tabhandle, &ps);
	
		// Double-buffering start
		auto memdc = ::CreateCompatibleDC(hdc);
		auto hbmbuf = ::CreateCompatibleBitmap(hdc, this->size.x, this->size.y);
		auto hbmold = ::SelectObject(memdc, hbmbuf);

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
			yNewPos = std::max(0, yNewPos);
			yNewPos = std::min(this->yMaxScroll, yNewPos);

			if (yNewPos == this->page)
			{
				break;
			}

			this->page = yNewPos;

			this->second.pageLoad(this->page + 1);
			::InvalidateRect(this->tabhandle, nullptr, true);

			SCROLLINFO si{};
			si.cbSize = sizeof si;
			si.fMask  = SIF_POS;
			si.nPos   = this->page;
			::SetScrollInfo(this->tabhandle, SB_VERT, &si, true);
		}
		break;
	}
	case WM_COMMAND:
		::SendMessageW(this->parent, WM_COMMAND, wp, lp);
		break;
	case WM_SIZE:
	{
		auto r = w::getCliR(this->tabhandle);
		this->size = { r.right - r.left, r.bottom - r.top };

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
		break;
	}
	case WM_CLOSE:
		this->~TabObject();
		break;
	default:
		return ::DefWindowProcW(this->tabhandle, uMsg, wp, lp);
	}

	return 0;
}

pdfv::Tabs::Tabs(HWND hwnd, HINSTANCE hInst) noexcept
	: m_parent(hwnd)
{
	DEBUGPRINT("pdfv::Tabs::Tabs(%p, %p)\n", static_cast<void *>(hwnd), static_cast<void *>(hInst));
	auto r = w::getCliR(this->m_parent);
	this->m_tabshwnd = ::CreateWindowExW(
		0,
		WC_TABCONTROL,
		L"",
		WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED,
		0,
		0,
		r.right  - r.left,
		r.bottom - r.top,
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
	if (this->m_tabshwnd != nullptr)
	{
		::DestroyWindow(this->m_tabshwnd);
		this->m_tabshwnd = nullptr;
	}
	this->m_parent = nullptr;
	pdfv::Pdfium::free();
}

void pdfv::Tabs::resize(xy<int> newsize) noexcept
{
	this->m_size = newsize;
	if (!this->m_tabs.empty())
	{
		::MoveWindow(
			this->m_tabs[this->m_tabindex]->tabhandle,
			this->m_offset.x, this->m_offset.y,
			this->m_size.x - this->m_offset.x, this->m_size.y - this->m_offset.y,
			TRUE
		);
	}

	::MoveWindow(
		this->m_tabshwnd,
		this->m_pos.x,  this->m_pos.y,
		this->m_size.x, this->m_size.y,
		TRUE
	);
}
void pdfv::Tabs::move(xy<int> newpos) noexcept
{
	this->m_pos = newpos;
	::MoveWindow(
		this->m_tabshwnd,
		this->m_pos.x,  this->m_pos.y,
		this->m_size.x, this->m_size.y,
		FALSE
	);
}
void pdfv::Tabs::repaint() const noexcept
{
	::InvalidateRect(this->m_tabshwnd, nullptr, TRUE);
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

pdfv::Tabs::listtype::iterator pdfv::Tabs::insert(std::wstring_view title, const pdfv::ssize_t index)
{
	TCITEMW tie;
	tie.mask = TCIF_TEXT | TCIF_IMAGE;
	tie.iImage = -1;

	pdfv::Tabs::listtype::iterator ret;
	if (index == Tabs::endpos)
	{
		// Insert at the end
		this->m_tabs.emplace_back(std::make_unique<TabObject>(this->m_tabshwnd, MainWindow::mwnd.getHinst(), title));
		ret = this->m_tabs.end();
		--ret;
		tie.pszText = (*ret)->first.data();
		TabCtrl_InsertItem(this->m_tabshwnd, this->m_tabs.size() - 1, &tie);
	}
	else
	{
		ret = this->m_tabs.insert(
			this->m_tabs.begin() + index,
			std::make_unique<TabObject>(this->m_tabshwnd, MainWindow::mwnd.getHinst(), title)
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
	std::size_t index = 0;
	for (auto it = this->m_tabs.begin(); it != this->m_tabs.end(); ++it, ++index)
	{
		if ((*it)->first == title)
		{
			(*it)->~TabObject();
			this->m_tabs.erase(it);
			TabCtrl_DeleteItem(this->m_tabshwnd, index);

			break;
		}
	}
}
void pdfv::Tabs::remove(const pdfv::ssize_t index) noexcept
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
		if (index >= ssize_t(this->m_tabs.size()))
		{
			return;
		}
		auto it = this->m_tabs.begin() + index;
		this->m_tabs.erase(it);
		TabCtrl_DeleteItem(this->m_tabshwnd, index);
	}
}
pdfv::Tabs::listtype::iterator pdfv::Tabs::rename(std::wstring_view title, const pdfv::ssize_t index)
{
	assert(!this->m_tabs.empty());

	TCITEMW tie;
	tie.mask = TCIF_TEXT | TCIF_IMAGE;
	tie.iImage = -1;

	if (index == Tabs::endpos)
	{
		// Rename the end
		auto it = this->m_tabs.end();
		--it;

		(*it)->first.assign(title);
		(*it)->first += pdfv::Tabs::padding;
		tie.pszText = (*it)->first.data();
		TabCtrl_SetItem(this->m_tabshwnd, this->m_tabs.size() - 1, &tie);
		return it;
	}
	else
	{
		auto it = this->m_tabs.begin() + index;

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
	auto oldidx = this->m_tabindex;
	this->m_tabindex = TabCtrl_GetCurSel(this->m_tabshwnd);
	
	this->m_tabs[oldidx]->show(false);
	this->resize(this->m_size);
	this->m_tabs[this->m_tabindex]->show();
}
