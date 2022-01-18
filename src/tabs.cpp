#include "tabs.hpp"
#include "mainwindow.hpp"

#include <unordered_map>

namespace pdfv
{
	struct TabProcInfo
	{
		pdfv::Pdfium * pdf{ nullptr };

		int yMaxScroll{};
		int yMinScroll{};
		int page{};

		TabProcInfo() noexcept = default;
		TabProcInfo(pdfv::Pdfium * l) noexcept
			: pdf(l)
		{
		}
	};
	static std::unordered_map<HWND, pdfv::TabProcInfo> info;

	[[nodiscard]] static inline HWND createTabobjectHWND(const HWND hwnd, const HINSTANCE hInst) noexcept
	{
		RECT r{};
		::GetClientRect(hwnd, &r);
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
			nullptr
		);
	}
}

pdfv::TabObject::TabObject(HWND tabshwnd, HINSTANCE hInst) noexcept
{
	this->tabhandle = pdfv::createTabobjectHWND(tabshwnd, hInst);
	pdfv::info[this->tabhandle] = TabProcInfo(this->second.get());
}
pdfv::TabObject::TabObject(HWND tabshwnd, HINSTANCE hInst, std::wstring_view V1, pdfv::Pdfium && V2)
	: first(V1), second(new Pdfium(std::move(V2)))
{
	this->first    += pdfv::Tabs::padding;
	this->tabhandle = pdfv::createTabobjectHWND(tabshwnd, hInst);
	pdfv::info[this->tabhandle] = TabProcInfo(this->second.get());
}
pdfv::TabObject::TabObject(HWND tabshwnd, HINSTANCE hInst, std::wstring && V1, pdfv::Pdfium && V2) noexcept
	: first(std::move(V1)), second(new Pdfium(std::move(V2)))
{
	this->first    += pdfv::Tabs::padding;
	this->tabhandle = pdfv::createTabobjectHWND(tabshwnd, hInst);
	pdfv::info[this->tabhandle] = TabProcInfo(this->second.get());
}
pdfv::TabObject::TabObject(TabObject && other) noexcept
	: first(std::move(other.first)), second(std::move(other.second)),
	closeButton(other.closeButton), tabhandle(other.tabhandle),
	handles(std::move(other.handles))
{
	other.closeButton = nullptr;
	other.tabhandle   = nullptr;
}
pdfv::TabObject & pdfv::TabObject::operator=(TabObject && other) noexcept
{
	this->~TabObject();

	this->first       = std::move(other.first);
	this->second      = std::move(other.second);
	this->closeButton = other.closeButton;
	this->tabhandle   = other.tabhandle;
	this->handles     = std::move(other.handles);

	other.closeButton = nullptr;
	other.tabhandle   = nullptr;

	return *this;
}
pdfv::TabObject::~TabObject() noexcept
{
	if (this->tabhandle != nullptr)
	{
		::DestroyWindow(this->tabhandle);
		info.erase(this->tabhandle);
		this->tabhandle = nullptr;
	}
	if (this->closeButton != nullptr)
	{
		::RemoveWindowSubclass(this->closeButton, &pdfv::TabObject::closeButtonProc, 1);
		::DestroyWindow(this->closeButton);
		this->closeButton = nullptr;
	}
}

void pdfv::TabObject::insert(HWND handle)
{
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
	::DestroyWindow(handle);
	this->handles.erase(handle);
	handle = nullptr;
}
void pdfv::TabObject::show() const noexcept
{
	::ShowWindow(this->tabhandle, SW_SHOW);
}
void pdfv::TabObject::hide() const noexcept
{
	::ShowWindow(this->tabhandle, SW_HIDE);
}
void pdfv::TabObject::updatePDF() const noexcept
{
	RECT r{};
	::GetClientRect(this->tabhandle, &r);
	::SendMessageW(this->tabhandle, WM_SIZE, 0, MAKELONG(r.right - r.left, r.bottom - r.top));
}

LRESULT CALLBACK pdfv::TabObject::tabProc(const HWND hwnd, const UINT uMsg, WPARAM wp, LPARAM lp)
{
	static HWND parent{};
	static pdfv::xy<int> sz;

	switch (uMsg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		auto hdc = ::BeginPaint(hwnd, &ps);
		
		if (auto it = info.find(hwnd); it != info.end())
		{
			auto & p = it->second;
			if (p.pdf != nullptr && p.pdf->pdfExists())
			{
				p.pdf->pageRender(hdc, { 0, 0 }, sz);
			}
		}

		::EndPaint(hwnd, &ps);
		break;
	}
	case WM_VSCROLL:
	{
		auto it = info.find(hwnd);
		if (it == info.end())
		{
			break;
		}

		auto & p = it->second;
		if (p.pdf != nullptr && p.pdf->pdfExists())
		{
			int yNewPos;

			switch (LOWORD(wp))
			{
			case SB_PAGEUP:
			case SB_LINEUP:
				yNewPos = p.page - 1;
				break;
			case SB_PAGEDOWN:
			case SB_LINEDOWN:
				yNewPos = p.page + 1;
				break;
			case SB_TOP:
				yNewPos = 0;
				break;
			case SB_BOTTOM:
				yNewPos = p.yMaxScroll;
				break;
			case SB_THUMBPOSITION:
				yNewPos = HIWORD(wp);
				break;
			case SB_THUMBTRACK:
			{
				SCROLLINFO si{};
				si.cbSize = sizeof si;
				si.fMask  = SIF_TRACKPOS;
				::GetScrollInfo(hwnd, SB_VERT, &si);

				yNewPos = si.nTrackPos;
				break;
			}
			default:
				yNewPos = p.page;
			}
			yNewPos = std::max(0, yNewPos);
			yNewPos = std::min(p.yMaxScroll, yNewPos);

			if (yNewPos == p.page)
			{
				break;
			}

			p.page = yNewPos;

			p.pdf->pageLoad(p.page + 1);
			::InvalidateRect(hwnd, nullptr, true);

			SCROLLINFO si;
			si.cbSize = sizeof si;
			si.fMask = SIF_POS;
			si.nPos = p.page;
			::SetScrollInfo(hwnd, SB_VERT, &si, true);
		}
		break;
	}
	case WM_COMMAND:
		::SendMessageW(parent, WM_COMMAND, wp, lp);
		break;
	case WM_SIZE:
	{
		RECT r{};
		::GetClientRect(hwnd, &r);
		sz = { r.right - r.left, r.bottom - r.top };

		auto it = info.find(hwnd);
		if (it == info.end())
		{
			break;
		}

		auto & p = it->second;
		if (p.pdf != nullptr && p.pdf->pdfExists())
		{
			p.yMaxScroll = p.pdf->pageGetCount() - 1;
			p.page = std::min(int(p.pdf->pageGetNum()) - 1, p.yMaxScroll);

			SCROLLINFO si{};
			si.cbSize = sizeof si;
			si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
			si.nMin   = p.yMinScroll;
			si.nMax   = p.yMaxScroll;
			si.nPage  = 1;
			si.nPos   = p.page;
			::SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		}
		break;
	}
	case WM_CLOSE:
		::DestroyWindow(hwnd);
		break;
	case WM_CREATE:
		parent = ::GetParent(hwnd);
		break;
	default:
		return ::DefWindowProcW(hwnd, uMsg, wp, lp);
	}

	return 0;
}

LRESULT CALLBACK pdfv::TabObject::closeButtonProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wp,
	LPARAM lp,
	[[maybe_unused]] UINT_PTR uIdSubClass,
	[[maybe_unused]] DWORD_PTR dwRefData
) noexcept
{
	switch (uMsg)
	{
	case WM_LBUTTONUP:
	{
		// Checks if mouse is in the right position
		POINT p;
		::GetCursorPos(&p);
		if (::WindowFromPoint(p) == hwnd)
		{
			::SendMessageW(
				MainWindow::mwnd.getHwnd(),
				WM_COMMAND,
				reinterpret_cast<WPARAM>(GetMenu(hwnd)),
				0
			);
		}
		break;
	}
	}
	return ::DefSubclassProc(hwnd, uMsg, wp, lp);
}

pdfv::Tabs::Tabs() noexcept
{
	pdfv::Pdfium::init();
}
pdfv::Tabs::Tabs(HWND hwnd, HINSTANCE hInst) noexcept
	: m_parent(hwnd)
{
	RECT r{};
	::GetClientRect(this->m_parent, &r);
	this->m_tabshwnd = ::CreateWindowExW(
		0,
		WC_TABCONTROL,
		L"",
		WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_TABSTOP,
		0,
		0,
		r.right,
		r.bottom,
		this->m_parent,
		nullptr,
		hInst,
		nullptr
	);
}
pdfv::Tabs::Tabs(pdfv::Tabs && other) noexcept
	: m_parent(other.m_parent), m_tabshwnd(other.m_tabshwnd), m_pos(std::move(other.m_pos)),
	m_size(std::move(other.m_size)), m_tabs(std::move(other.m_tabs)),
	m_tabindex(other.m_tabindex)
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
	pdfv::Pdfium::free();
	if (this->m_tabshwnd != nullptr)
	{
		::DestroyWindow(this->m_tabshwnd);
		this->m_tabshwnd = nullptr;
	}
}

[[nodiscard]] HWND pdfv::Tabs::createCloseButton(RECT sz, HMENU menu) const noexcept
{
	auto btn = ::CreateWindowExW(
		0,
		L"button",
		L"X",
		WS_VISIBLE | WS_CLIPSIBLINGS | WS_CHILD,
		sz.left,
		sz.top,
		sz.right  - sz.left,
		sz.bottom - sz.top,
		this->m_tabshwnd,
		menu,
		pdfv::MainWindow::mwnd.getHinst(),
		nullptr
	);
	if (btn == nullptr) [[unlikely]]
	{
		return nullptr;
	}

	::SendMessageW(
		btn,
		WM_SETFONT,
		reinterpret_cast<WPARAM>(pdfv::MainWindow::mwnd.getDefaultFont()),
		true
	);
	::SetWindowSubclass(btn, &pdfv::TabObject::closeButtonProc, 1, 0);
	return btn;
}

void pdfv::Tabs::resize(xy<int> newsize) noexcept
{
	this->m_size = newsize;
	if (!this->m_tabs.empty())
	{
		::MoveWindow(
			this->m_tabs[this->m_tabindex].tabhandle,
			this->m_offset.x, this->m_offset.y,
			this->m_size.x - this->m_offset.x, this->m_size.y - this->m_offset.y,
			TRUE
		);
	}

	::MoveWindow(
		this->m_tabshwnd,
		this->m_pos.x, this->m_pos.y,
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
	::InvalidateRect(this->m_tabshwnd, nullptr, true);
}
void pdfv::Tabs::moveCloseButtons() const noexcept
{
	for (std::size_t i = 0; i < this->m_tabs.size(); ++i)
	{
		this->moveCloseButton(i);
	}
}
void pdfv::Tabs::moveCloseButton(pdfv::ssize_t index) const noexcept
{
	if (index == pdfv::Tabs::endpos)
	{
		// Move the end
		index = this->m_tabs.size() - 1;
	}

	RECT r{};
	TabCtrl_GetItemRect(this->m_tabshwnd, index, &r);
	::MoveWindow(
		this->m_tabs[index].closeButton,
		r.right - dip(s_cCloseButtonSz.x + 2, dpi.x),
		(r.top + r.bottom) / 2 - dip(s_cCloseButtonSz.y, dpi.y) / 2,
		dip(s_cCloseButtonSz.x, dpi.x),
		dip(s_cCloseButtonSz.y, dpi.y),
		FALSE
	);
}
void pdfv::Tabs::updateCloseButtons() const noexcept
{
	for (const auto & i : this->m_tabs)
	{
		::InvalidateRect(i.closeButton, nullptr, TRUE);
	}
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
		this->m_tabs.emplace_back(this->m_tabshwnd, MainWindow::mwnd.getHinst(), title);
		ret = this->m_tabs.end();
		--ret;
		tie.pszText = const_cast<LPWSTR>(ret->first.c_str());
		TabCtrl_InsertItem(this->m_tabshwnd, this->m_tabs.size() - 1, &tie);
		ret->closeButton = this->createCloseButton({}, reinterpret_cast<HMENU>(this->m_tabs.size() + IDM_LIMIT - 1));
		this->moveCloseButton(Tabs::endpos);
	}
	else
	{
		ret = this->m_tabs.insert(
			std::next(this->m_tabs.begin(), index),
			TabObject(this->m_tabshwnd, MainWindow::mwnd.getHinst(), title)
		);
		tie.pszText = const_cast<LPWSTR>(ret->first.c_str());
		TabCtrl_InsertItem(this->m_tabshwnd, index, &tie);
		// Update indexes
		ret->closeButton = this->createCloseButton({}, reinterpret_cast<HMENU>(index + IDM_LIMIT));
		
		this->moveCloseButtons();
		for (std::size_t i = index + 1; i < this->m_tabs.size(); ++i)
		{
			::SetWindowLongW(this->m_tabs[i].closeButton, GWL_ID, i + IDM_LIMIT);
		}
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
		if (it->first == title)
		{
			it->~TabObject();
			this->m_tabs.erase(it);
			TabCtrl_DeleteItem(this->m_tabshwnd, index);
			moveCloseButtons();
			for (std::size_t i = index; i < this->m_tabs.size(); ++i)
			{
				::SetWindowLongW(this->m_tabs[i].closeButton, GWL_ID, i + IDM_LIMIT);
			}

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
		moveCloseButtons();
		
		for (std::size_t i = index; i < this->m_tabs.size(); ++i)
		{
			::SetWindowLongW(this->m_tabs[i].closeButton, GWL_ID, i + IDM_LIMIT);
		}
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
		auto it = std::next(this->m_tabs.end(), -1);
		it->first.assign(title);
		it->first += pdfv::Tabs::padding;
		tie.pszText = const_cast<LPWSTR>(it->first.c_str());
		TabCtrl_SetItem(this->m_tabshwnd, this->m_tabs.size() - 1, &tie);
		moveCloseButton(Tabs::endpos);
		return it;
	}
	else
	{
		auto it = std::next(this->m_tabs.begin(), index);
		it->first.assign(title);
		it->first += pdfv::Tabs::padding;
		tie.pszText = const_cast<LPWSTR>(it->first.c_str());
		TabCtrl_SetItem(this->m_tabshwnd, index, &tie);
		moveCloseButtons();
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
	
	this->m_tabs[oldidx].hide();
	this->resize(this->m_size);
	this->m_tabs[this->m_tabindex].show();
}
