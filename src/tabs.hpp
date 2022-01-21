#pragma once
#include "common.hpp"
#include "lib.hpp"

#include <list>
#include <utility>
#include <set>

namespace pdfv
{
	// Declare before use in be-friending
	class Tabs;
	class MainWindow;

	class TabObject
	{
	public:
		std::wstring first;
		pdfv::Pdfium second;

		TabObject() noexcept = delete;
		TabObject(HWND tabshwnd, HINSTANCE hinst) noexcept;
		TabObject(HWND tabshwnd, HINSTANCE hInst, std::wstring_view V1, pdfv::Pdfium && V2 = pdfv::Pdfium());
		TabObject(HWND tabshwnd, HINSTANCE hInst, std::wstring && V1, pdfv::Pdfium && V2 = pdfv::Pdfium()) noexcept;
		TabObject(const TabObject & other) = delete;
		TabObject(TabObject && other) noexcept;
		TabObject & operator=(const TabObject & other) = delete;
		TabObject & operator=(TabObject && other) noexcept;
		~TabObject() noexcept;
		
		//
		//	Inserts handle to the tab and sets its parent to the tab if needed
		//
		void insert(HWND handle);
		//
		//	Inserts and constructs in-place an element to the tab
		//
		const HWND & insert(
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
		void remove(HWND & handle) noexcept;
		//
		//	Shows the tab window
		//
		void show(bool visible = true) const noexcept;
		//
		//	Informs the WndProc of the tab to update it's scrollbars
		//
		void updatePDF() const noexcept;

	private:
		// Private variables
		HWND tabhandle{ nullptr }, parent{ nullptr };
		xy<int> size;

		int yMaxScroll{};
		int yMinScroll{};
		int page{};

		std::set<HWND> handles;

		friend class pdfv::Tabs;
		friend class pdfv::MainWindow;

		static LRESULT CALLBACK tabProcHub(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp);
		LRESULT tabProc(UINT uMsg, WPARAM wp, LPARAM lp);
	};

	class Tabs
	{
	public:
		static constexpr ssize_t endpos{ -1 };
		static inline const std::wstring padding{ L"      " };
		static inline const std::wstring defaulttitle{ L"unopened" };
		static inline const std::wstring defaulttitlepadded{ defaulttitle + padding };

	private:
		HWND m_parent{ nullptr };
		HWND m_tabshwnd{ nullptr };
		xy<int> m_pos;
		xy<int> m_size;
		xy<int> m_offset;

		using listtype = std::vector<std::unique_ptr<pdfv::TabObject>>;

		listtype m_tabs;
		ssize_t m_tabindex{};

		friend class pdfv::MainWindow;

		static constexpr xy<int> s_cCloseButtonSz{ 20, 20 };

	public:
		Tabs() noexcept = default;
		//
		//	Initializes a tab by using parent's HWND and HINSTANCE
		//
		Tabs(HWND hwnd, HINSTANCE hInst) noexcept;
		Tabs(const Tabs & other) = delete;
		Tabs(Tabs && other) noexcept;
		Tabs & operator=(const Tabs & other) = delete;
		Tabs & operator=(Tabs && other) noexcept;
		~Tabs() noexcept;

		//
		//	Returns const reference to the tabs' handle
		//
		[[nodiscard]] constexpr const HWND & getHandle() const noexcept
		{
			return this->m_tabshwnd;
		}

		//
		//	Resizes the whole tab control
		//
		void resize(xy<int> newsize) noexcept;
		//
		//	Moves the tab control to a new position
		//
		void move(xy<int> newpos) noexcept;
		//
		//	Redraws the tab control
		//
		void repaint() const noexcept;
		/**
		 * @brief Calculates the rectangle of tab close button, respective to tabs rectangle
		 * 
		 * @param itemRect Tabs rectangle
		 * @return RECT Calculated rectangle
		 */
		[[nodiscard]] static RECT s_calcCloseButton(RECT itemRect) noexcept;
		//
		//	Inserts a new tab at a given index to the tab control
		//	Default index is tabs::endpos
		//
		listtype::iterator insert(std::wstring_view title, const ssize_t index = Tabs::endpos);
		//
		//	Removes a tab with the specified name
		//
		void remove(std::wstring_view title) noexcept;
		//
		//	Removes a tab at the specified index
		//	Default index is tabs::endpos
		//
		void remove(const ssize_t index = Tabs::endpos) noexcept;
		//
		//	Renames a tab at the specified index
		//	Default index is tabs::endpos
		//
		listtype::iterator rename(std::wstring_view title, const ssize_t index = Tabs::endpos);
		//
		//	Returns tab's name at the specified index
		//
		[[nodiscard]] std::wstring_view getName(const ssize_t index = Tabs::endpos) const noexcept;
		//
		//	Selects a tab at the specified index
		//	Default index is tabs::endpos
		//
		void select(const ssize_t index = Tabs::endpos) noexcept;
		//
		//	Returns the number of tabs currently present
		//
		[[nodiscard]] std::size_t size() const noexcept;
		//
		//	Handles the tab control active tab changes
		//
		void selChange() noexcept;


	};
}
