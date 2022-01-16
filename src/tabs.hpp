#pragma once
#include "common.hpp"
#include "lib.hpp"

#include <list>
#include <utility>
#include <set>

namespace pdfv {
	
	// Declare before use in friending
	class tabs;
	class main_window;

	class tabobject {
	public:
		std::wstring first;
		pdfv::lib second;
		HWND closeButton{};

		tabobject() noexcept = delete;
		tabobject(HWND tabshwnd, HINSTANCE hinst) noexcept;
		tabobject(HWND tabshwnd, HINSTANCE hInst, std::wstring const& V1, pdfv::lib&& V2 = pdfv::lib());
		tabobject(HWND tabshwnd, HINSTANCE hInst, std::wstring_view V1, pdfv::lib&& V2 = pdfv::lib());
		tabobject(HWND tabshwnd, HINSTANCE hInst, std::wstring&& V1, pdfv::lib&& V2 = pdfv::lib()) noexcept;
		tabobject(tabobject const& other) = delete;
		tabobject(tabobject&& other) noexcept;
		tabobject& operator=(tabobject const& other) = delete;
		tabobject& operator=(tabobject&& other) noexcept;
		~tabobject() noexcept;
		
		//
		//	Inserts handle to the tab and sets its parent to the tab if needed
		//
		void Insert(HWND handle);
		//
		//	Inserts and constructs in-place an element to the tab
		//
		HWND const& Insert(
			DWORD dwExStyle,
			std::wstring_view className,
			std::wstring_view windowName,
			DWORD dwStyle,
			xy<int> pos = { CW_USEDEFAULT, CW_USEDEFAULT },
			xy<int> size = { CW_USEDEFAULT, CW_USEDEFAULT },
			HMENU menu = nullptr,
			HINSTANCE hinst = nullptr
		);
		//
		//	Removes specified handle from the tab and destroys it
		//	Sets the given handle to nullptr
		//
		void Remove(HWND& handle) noexcept;
		//
		//	Shows the tab window
		//
		void Show() const noexcept;
		//
		//	Hides the tab window
		//
		void Hide() const noexcept;
		//
		//	Informs the WndProc of the tab to update it's scrollbars
		//
		void UpdatePDF() const noexcept;

	private:
		// Private variables
		HWND tabhandle{};
		std::set<HWND> handles;

		friend class pdfv::tabs;
		friend class pdfv::main_window;

		friend int WINAPI ::wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int);
		static LRESULT CALLBACK TabProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp) noexcept;
		static LRESULT CALLBACK CloseButtonProc(
			HWND hwnd,
			UINT uMsg,
			WPARAM wp,
			LPARAM lp,
			[[maybe_unused]] UINT_PTR uIdSubClass,
			[[maybe_unused]] DWORD_PTR dwRefData
		) noexcept;
	};

	class tabs {
	public:
		static inline const ssize_t endpos{ -1 };
		static inline const std::wstring padding{ L"      " };
		static inline const std::wstring defaulttitle{ L"unopened" };
		static inline const std::wstring defaulttitlepadded{ defaulttitle + padding };

	private:
		HWND m_parent{};
		HWND m_tabshwnd{};
		xy<int> m_pos;
		xy<int> m_size;
		xy<int> m_offset;

		using listtype = std::vector<pdfv::tabobject>;

		listtype m_tabs;
		ssize_t m_tabindex{};

		friend class pdfv::main_window;

		//
		//	Creates the close button for the tab with appropriate size
		//
		[[nodiscard]] HWND CreateCloseButton(RECT rect, HMENU menu) const noexcept;

		static inline const xy<int> s_cCloseButtonSz{ 20, 20 };

	public:
		tabs() noexcept;
		//
		//	Initializes a tab by using parent's HWND and HINSTANCE
		//
		tabs(HWND hwnd, HINSTANCE hInst) noexcept;
		tabs(tabs const& other) = delete;
		tabs(tabs&& other) noexcept;
		tabs& operator=(tabs const& other) = delete;
		tabs& operator=(tabs&& other) noexcept;
		~tabs() noexcept;

		//
		//	Returns const reference to the tabs' handle
		//
		[[nodiscard]] constexpr HWND const& GetHandle() const noexcept
		{
			return m_tabshwnd;
		}

		//
		//	Resizes the whole tab control
		//
		void Resize(xy<int> newsize) noexcept;
		//
		//	Moves the tab control to a new position
		//
		void Move(xy<int> newpos) noexcept;
		//
		//	Redraws the tab control
		//
		void Repaint() const noexcept;
		//
		//	Moves the close buttons to the appropriate position of the tab
		//
		void MoveCloseButtons() const noexcept;
		//
		//	Moves the close button at the specified index to the appropriate
		//	position of the tab
		//	If index is tabs::endpos, moves the last tab's close button
		//
		void MoveCloseButton(pdfv::ssize_t index) const noexcept;
		//
		//	Updates close buttons on screen
		//
		void UpdateCloseButtons() const noexcept;
		//
		//	Inserts a new tab at a given index to the tab control
		//	Default index is tabs::endpos
		//
		listtype::iterator Insert(std::wstring_view title, const ssize_t index = tabs::endpos);
		//
		//	Removes a tab with the specified name
		//
		void Remove(std::wstring_view title) noexcept;
		//
		//	Removes a tab at the specified index
		//	Default index is tabs::endpos
		//
		void Remove(const ssize_t index = tabs::endpos) noexcept;
		//
		//	Renames a tab at the specified index
		//	Default index is tabs::endpos
		//
		listtype::iterator Rename(std::wstring_view title, const ssize_t index = tabs::endpos);
		//
		//	Returns tab's name at the specified index
		//
		[[nodiscard]] std::wstring_view GetName(const ssize_t index = tabs::endpos) const noexcept;
		//
		//	Selects a tab at the specified index
		//	Default index is tabs::endpos
		//
		void Select(const ssize_t index = tabs::endpos) noexcept;
		//
		//	Returns the number of tabs currently present
		//
		[[nodiscard]] std::size_t Size() const noexcept;
		//
		//	Handles the tab control active tab changes
		//
		void SelChange() noexcept;
	};
}
