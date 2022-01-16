#include "tabs.hpp"
#include "main_window.hpp"

#include <unordered_map>

namespace pdfv {
	struct TabProcInfo {
		pdfv::lib* pdf{};

		int yMaxScroll{};
		int yMinScroll{};
		int page{};

		TabProcInfo() noexcept = default;
		TabProcInfo(pdfv::lib* l) noexcept
			: pdf(l)
		{}
	};
	static std::unordered_map<HWND, pdfv::TabProcInfo> info;

	[[nodiscard]] static inline HWND CreateTabobjectHWND(const HWND hwnd, const HINSTANCE hInst) noexcept
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

pdfv::tabobject::tabobject(HWND tabshwnd, HINSTANCE hInst) noexcept
{
	tabhandle = pdfv::CreateTabobjectHWND(tabshwnd, hInst);
	pdfv::info[tabhandle].pdf = &second;
}
pdfv::tabobject::tabobject(HWND tabshwnd, HINSTANCE hInst, std::wstring const& V1, pdfv::lib&& V2)
	: first(V1), second(std::move(V2))
{
	first += pdfv::tabs::padding;
	tabhandle = pdfv::CreateTabobjectHWND(tabshwnd, hInst);
	pdfv::info[tabhandle] = TabProcInfo(&second);
}
pdfv::tabobject::tabobject(HWND tabshwnd, HINSTANCE hInst, std::wstring_view V1, pdfv::lib&& V2)
	: first(V1), second(std::move(V2))
{
	first += pdfv::tabs::padding;
	tabhandle = pdfv::CreateTabobjectHWND(tabshwnd, hInst);
	pdfv::info[tabhandle] = TabProcInfo(&second);
}
pdfv::tabobject::tabobject(HWND tabshwnd, HINSTANCE hInst, std::wstring&& V1, pdfv::lib&& V2) noexcept
	: first(std::move(V1)), second(std::move(V2))
{
	first += pdfv::tabs::padding;
	tabhandle = pdfv::CreateTabobjectHWND(tabshwnd, hInst);
	pdfv::info[tabhandle] = TabProcInfo(&second);
}
pdfv::tabobject::tabobject(tabobject&& other) noexcept
	: first(std::move(other.first)), second(std::move(other.second)),
	closeButton(other.closeButton), tabhandle(other.tabhandle),
	handles(std::move(other.handles))
{
	pdfv::info[tabhandle].pdf = &second;
	other.closeButton = nullptr;
	other.tabhandle = nullptr;
}
pdfv::tabobject& pdfv::tabobject::operator=(tabobject&& other) noexcept
{
	first = std::move(other.first);
	tabhandle = other.tabhandle;
	second = std::move(other.second);
	pdfv::info[tabhandle].pdf = &second;
	closeButton = other.closeButton;
	handles = std::move(other.handles);

	other.closeButton = nullptr;
	other.tabhandle = nullptr;

	return *this;
}
pdfv::tabobject::~tabobject() noexcept
{
	for (auto i : handles)
		::DestroyWindow(i);
	::RemoveWindowSubclass(closeButton, &pdfv::tabobject::CloseButtonProc, 1);
	::DestroyWindow(closeButton);
	::DestroyWindow(tabhandle);
	info.erase(tabhandle);
}

void pdfv::tabobject::Insert(HWND handle)
{
	auto p = ::GetParent(handle);
	if (p != tabhandle) {
		::SetParent(handle, tabhandle);
	}
	handles.insert(handle);
}
HWND const& pdfv::tabobject::Insert(
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
	return *(handles.emplace(::CreateWindowExW(
		dwExStyle,
		className.data(),
		windowName.data(),
		dwStyle,
		pos.x,
		pos.y,
		size.x,
		size.y,
		tabhandle,
		menu,
		hinst,
		nullptr
	)).first);
}
void pdfv::tabobject::Remove(HWND& handle) noexcept
{
	::DestroyWindow(handle);
	handles.erase(handle);
	handle = nullptr;
}
void pdfv::tabobject::Show() const noexcept
{
	::ShowWindow(tabhandle, SW_SHOW);
}
void pdfv::tabobject::Hide() const noexcept
{
	::ShowWindow(tabhandle, SW_HIDE);
}
void pdfv::tabobject::UpdatePDF() const noexcept
{
	RECT r{};
	::GetClientRect(tabhandle, &r);
	::SendMessageW(tabhandle, WM_SIZE, 0, MAKELONG(r.right - r.left, r.bottom - r.top));
}

LRESULT CALLBACK pdfv::tabobject::TabProc(const HWND hwnd, const UINT uMsg, WPARAM wp, LPARAM lp) noexcept
{
	static HWND parent{};
	static pdfv::xy<int> sz;

	switch (uMsg) {
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		[[maybe_unused]] auto hdc = ::BeginPaint(hwnd, &ps);
		
		auto& p = info.at(hwnd);
		if (p.pdf != nullptr && p.pdf->PdfExists())
			p.pdf->RenderPage(hdc, { 0, 0 }, sz);

		::EndPaint(hwnd, &ps);
		break;
	}
	case WM_VSCROLL:
	{
		auto& p = info.at(hwnd);
		if (p.pdf != nullptr && p.pdf->PdfExists()) {
			int yNewPos;

			switch (LOWORD(wp)) {
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
				SCROLLINFO si;
				si.cbSize = sizeof si;
				si.fMask = SIF_TRACKPOS;
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
				break;

			p.page = yNewPos;

			p.pdf->PageLoad(p.page + 1);
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
		sz = { LOWORD(lp), HIWORD(lp) };
		auto& p = info.at(hwnd);
		if (p.pdf != nullptr && p.pdf->PdfExists()) {
			p.yMaxScroll = p.pdf->GetPageCount() - 1;
			p.page = std::min(int(p.pdf->GetPageNum()) - 1, p.yMaxScroll);

			SCROLLINFO si;
			si.cbSize = sizeof si;
			si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
			si.nMin = p.yMinScroll;
			si.nMax = p.yMaxScroll;
			si.nPage = 1;
			si.nPos = p.page;
			::SetScrollInfo(hwnd, SB_VERT, &si, true);
		}
		break;
	}
	case WM_CLOSE:
		::DestroyWindow(hwnd);
		break;
	case WM_CREATE:
		parent = ::GetParent(hwnd);
		info.operator[](hwnd) = TabProcInfo();
		break;
	default:
		return ::DefWindowProcW(hwnd, uMsg, wp, lp);
	}
	return 0;
}

LRESULT CALLBACK pdfv::tabobject::CloseButtonProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wp,
	LPARAM lp,
	[[maybe_unused]] UINT_PTR uIdSubClass,
	[[maybe_unused]] DWORD_PTR dwRefData
) noexcept
{
	switch (uMsg) {
	case WM_LBUTTONUP:
	{
		// Checks if mouse is in the right position
		POINT p;
		::GetCursorPos(&p);
		if (::WindowFromPoint(p) == hwnd)
			::SendMessageW(
				main_window::mwnd.GetHWND(),
				WM_COMMAND,
				reinterpret_cast<WPARAM>(GetMenu(hwnd)),
				0
			);
		break;
	}
	}
	return ::DefSubclassProc(hwnd, uMsg, wp, lp);
}

pdfv::tabs::tabs() noexcept
{
	pdfv::lib::Init();
}
pdfv::tabs::tabs(HWND hwnd, HINSTANCE hInst) noexcept
	: m_parent(hwnd)
{
	RECT r{};
	::GetClientRect(m_parent, &r);
	m_tabshwnd = ::CreateWindowExW(
		0,
		WC_TABCONTROL,
		L"",
		WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_TABSTOP,
		0,
		0,
		r.right,
		r.bottom,
		m_parent,
		nullptr,
		hInst,
		nullptr
	);
}
pdfv::tabs::tabs(pdfv::tabs&& other) noexcept
	: m_parent(other.m_parent), m_tabshwnd(other.m_tabshwnd), m_pos(std::move(other.m_pos)),
	m_size(std::move(other.m_size)), m_tabs(std::move(other.m_tabs)),
	m_tabindex(other.m_tabindex)
{
	other.m_parent = nullptr;
	other.m_tabshwnd = nullptr;
	other.m_tabindex = 0;
}
pdfv::tabs& pdfv::tabs::operator=(pdfv::tabs&& other) noexcept
{
	m_parent = other.m_parent;
	m_tabshwnd = other.m_tabshwnd;
	m_pos = std::move(other.m_pos);
	m_size = std::move(other.m_size);
	m_tabs = std::move(other.m_tabs);
	m_tabindex = other.m_tabindex;

	other.m_parent = nullptr;
	other.m_tabshwnd = nullptr;
	other.m_tabindex = 0;

	return *this;
}
pdfv::tabs::~tabs() noexcept
{
	m_tabs.clear();
	pdfv::lib::Destroy();
	::DestroyWindow(m_tabshwnd);
}

[[nodiscard]] HWND pdfv::tabs::CreateCloseButton(RECT sz, HMENU menu) const noexcept
{
	auto ret = ::CreateWindowExW(
		0,
		L"button",
		L"X",
		WS_VISIBLE | WS_CLIPSIBLINGS | WS_CHILD,
		sz.left,
		sz.top,
		sz.right  - sz.left,
		sz.bottom - sz.top,
		m_tabshwnd,
		menu,
		pdfv::main_window::mwnd.GetHINST(),
		nullptr
	);
	::SendMessageW(
		ret,
		WM_SETFONT,
		reinterpret_cast<WPARAM>(pdfv::main_window::mwnd.GetDefaultFont()),
		true
	);
	::SetWindowSubclass(ret, &pdfv::tabobject::CloseButtonProc, 1, 0);
	return ret;
}

void pdfv::tabs::Resize(xy<int> newsize) noexcept
{
	m_size = newsize;
	if (!m_tabs.empty())
		::MoveWindow(
			m_tabs[m_tabindex].tabhandle,
			m_offset.x         , m_offset.y,
			m_size.x - m_offset.x, m_size.y - m_offset.y,
			true
		);

	::MoveWindow(m_tabshwnd, m_pos.x, m_pos.y, m_size.x, m_size.y, true);
}
void pdfv::tabs::Move(xy<int> newpos) noexcept
{
	m_pos = newpos;
	::MoveWindow(m_tabshwnd, m_pos.x, m_pos.y, m_size.x, m_size.y, false);
}
void pdfv::tabs::Repaint() const noexcept
{
	::InvalidateRect(m_tabshwnd, nullptr, true);
}
void pdfv::tabs::MoveCloseButtons() const noexcept
{
	RECT r;
	for (std::size_t i = 0; i < m_tabs.size(); ++i) {
		TabCtrl_GetItemRect(m_tabshwnd, i, &r);
		::MoveWindow(
			m_tabs[i].closeButton,
			r.right - DPI(s_cCloseButtonSz.x + 2, dpi.x),
			(r.top + r.bottom) / 2 - DPI(s_cCloseButtonSz.y, dpi.y) / 2,
			DPI(s_cCloseButtonSz.x, dpi.x),
			DPI(s_cCloseButtonSz.y, dpi.y),
			false
		);
	}
}
void pdfv::tabs::MoveCloseButton(pdfv::ssize_t index) const noexcept
{
	RECT r;
	if (index == pdfv::tabs::endpos) {
		// Move the end
		TabCtrl_GetItemRect(m_tabshwnd, m_tabs.size() - 1, &r);
		::MoveWindow(
			m_tabs.back().closeButton,
			r.right - DPI(s_cCloseButtonSz.x + 2, dpi.x),
			(r.top + r.bottom) / 2 - DPI(s_cCloseButtonSz.y, dpi.y) / 2,
			DPI(s_cCloseButtonSz.x, dpi.x),
			DPI(s_cCloseButtonSz.y, dpi.y),
			false
		);
	}
	else {
		TabCtrl_GetItemRect(m_tabshwnd, index, &r);
		::MoveWindow(
			m_tabs[index].closeButton,
			r.right - DPI(s_cCloseButtonSz.x + 2, dpi.x),
			(r.top + r.bottom) / 2 - DPI(s_cCloseButtonSz.y, dpi.y) / 2,
			DPI(s_cCloseButtonSz.x, dpi.x),
			DPI(s_cCloseButtonSz.y, dpi.y),
			false
		);
	}
}
void pdfv::tabs::UpdateCloseButtons() const noexcept
{
	for (auto const& i : m_tabs) {
		::InvalidateRect(i.closeButton, nullptr, true);
	}
}

pdfv::tabs::listtype::iterator pdfv::tabs::Insert(std::wstring_view title, const pdfv::ssize_t index)
{
	TCITEMW tie;
	tie.mask = TCIF_TEXT | TCIF_IMAGE;
	tie.iImage = -1;

	pdfv::tabs::listtype::iterator ret;
	if (index == tabs::endpos) {
		// Insert at the end
		m_tabs.emplace_back(m_tabshwnd, main_window::mwnd.GetHINST(), title);
		ret = std::next(m_tabs.end(), -1);
		tie.pszText = const_cast<LPWSTR>(ret->first.c_str());
		TabCtrl_InsertItem(m_tabshwnd, m_tabs.size() - 1, &tie);
		ret->closeButton = CreateCloseButton(
			{},
			reinterpret_cast<HMENU>(m_tabs.size() + IDM_LIMIT - 1)
		);
		MoveCloseButton(tabs::endpos);
	}
	else {
		ret = m_tabs.insert(
			std::next(m_tabs.begin(), index),
			tabobject(m_tabshwnd, main_window::mwnd.GetHINST(),
			title)
		);
		tie.pszText = const_cast<LPWSTR>(ret->first.c_str());
		TabCtrl_InsertItem(m_tabshwnd, index, &tie);
		// Update indexes
		ret->closeButton = CreateCloseButton(
			{},
			reinterpret_cast<HMENU>(index + IDM_LIMIT)
		);
		
		MoveCloseButtons();
		for (std::size_t i = index + 1; i < m_tabs.size(); ++i) {
			::SetWindowLongW(m_tabs[i].closeButton, GWL_ID, i + IDM_LIMIT);
		}
	}
	if (!m_offset.y)
		m_offset.y = m_pos.y + DPI(23, dpi.y);

	return ret;
}
void pdfv::tabs::Remove(std::wstring_view title) noexcept
{
	std::size_t index = 0;
	for (auto it = m_tabs.begin(); it != m_tabs.end(); ++it, ++index) {
		if (it->first == title) {
			it->~tabobject();
			m_tabs.erase(it);
			TabCtrl_DeleteItem(m_tabshwnd, index);
			MoveCloseButtons();
			for (std::size_t i = index; i < m_tabs.size(); ++i) {
				::SetWindowLongW(m_tabs[i].closeButton, GWL_ID, i + IDM_LIMIT);
			}

			break;
		}
	}
}
void pdfv::tabs::Remove(const pdfv::ssize_t index) noexcept
{
	if (index == tabs::endpos) {
		// Remove from the end
		m_tabs.pop_back();
		TabCtrl_DeleteItem(m_tabshwnd, m_tabs.size());
	}
	else {
		auto it = std::next(m_tabs.begin(), index);
		it->~tabobject();
		m_tabs.erase(it);
		TabCtrl_DeleteItem(m_tabshwnd, index);
		MoveCloseButtons();
		
		for (std::size_t i = index; i < m_tabs.size(); ++i) {
			::SetWindowLongW(m_tabs[i].closeButton, GWL_ID, i + IDM_LIMIT);
		}
	}
}
pdfv::tabs::listtype::iterator pdfv::tabs::Rename(std::wstring_view title, const pdfv::ssize_t index)
{
	assert(!m_tabs.empty());

	TCITEMW tie;
	tie.mask = TCIF_TEXT | TCIF_IMAGE;
	tie.iImage = -1;

	if (index == tabs::endpos) {
		// Rename the end
		auto it = std::next(m_tabs.end(), -1);
		it->first.assign(title);
		it->first += pdfv::tabs::padding;
		tie.pszText = const_cast<LPWSTR>(it->first.c_str());
		TabCtrl_SetItem(m_tabshwnd, m_tabs.size() - 1, &tie);
		MoveCloseButton(tabs::endpos);
		return it;
	}
	else {
		auto it = std::next(m_tabs.begin(), index);
		it->first.assign(title);
		it->first += pdfv::tabs::padding;
		tie.pszText = const_cast<LPWSTR>(it->first.c_str());
		TabCtrl_SetItem(m_tabshwnd, index, &tie);
		MoveCloseButtons();
		return it;
	}
}
[[nodiscard]] std::wstring_view pdfv::tabs::GetName(const pdfv::ssize_t index) const noexcept
{
	if (index == tabs::endpos)
		return m_tabs.back().first;
	else
		return m_tabs[index].first;
}
void pdfv::tabs::Select(const ssize_t index) noexcept
{
	if (index == tabs::endpos) {
		TabCtrl_SetCurSel(m_tabshwnd, m_tabs.size() - 1);
	}
	else {
		TabCtrl_SetCurSel(m_tabshwnd, index);
	}
	SelChange();
}
[[nodiscard]] std::size_t pdfv::tabs::Size() const noexcept
{
	return m_tabs.size();
}
void pdfv::tabs::SelChange() noexcept
{
	m_tabs[m_tabindex].Hide();
	m_tabindex = TabCtrl_GetCurSel(m_tabshwnd);
	Resize(m_size);
	m_tabs[m_tabindex].Show();
}
