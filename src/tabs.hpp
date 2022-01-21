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
		
		/**
		 * @brief Inserts handle to the taba and sets its parent to the tab if necessary
		 * 
		 * @param handle Window handle
		 */
		void insert(HWND handle);
		/**
		 * @brief Inserts and constructs in-place an element to the tab
		 * 
		 * @param dwExStyle Extended style
		 * @param className Window class name
		 * @param windowName Window title
		 * @param dwStyle Style
		 * @param pos Window position value pair, relative to the tab, CW_USEDEFAULT by default
		 * @param size Window size value pair, CW_USEDEFAULT by default
		 * @param menu Window menu handle, nullptr by default
		 * @param hinst Module instance handle, nullptr by default
		 * @return const HWND& Constructed window handle
		 */
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
		/**
		 * @brief Remove handle from the tab and destroys it, sets given handle
		 * to nullptr
		 * 
		 * @param handle Window handle
		 */
		void remove(HWND & handle) noexcept;
		/**
		 * @brief Shows/hides the tab window
		 * 
		 * @param visible Determines the visibility of the window
		 */
		void show(bool visible = true) const noexcept;
		/**
		 * @brief Informs the window procedure of the tab to update its scrollbars
		 * 
		 */
		void updatePDF() const noexcept;

	private:
		HWND tabhandle{ nullptr }, parent{ nullptr };
		xy<int> size;

		int yMaxScroll{};
		int yMinScroll{};
		int page{};

		std::set<HWND> handles;

		friend class pdfv::Tabs;
		friend class pdfv::MainWindow;

		/**
		 * @brief "Hub" to tabs' window procedures
		 * 
		 */
		static LRESULT CALLBACK tabProcHub(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp);
		/**
		 * @brief "Real" tab's window procedure
		 * 
		 */
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
		/**
		 * @brief Initialises a tab by using parent's window handle and module handle
		 * 
		 * @param hwnd Parent's window handle
		 * @param hInst Module instance handle
		 */
		Tabs(HWND hwnd, HINSTANCE hInst) noexcept;
		Tabs(const Tabs & other) = delete;
		Tabs(Tabs && other) noexcept;
		Tabs & operator=(const Tabs & other) = delete;
		Tabs & operator=(Tabs && other) noexcept;
		~Tabs() noexcept;

		/**
		 * @return constexpr const HWND& Tab system window handle
		 */
		[[nodiscard]] constexpr const HWND & getHandle() const noexcept
		{
			return this->m_tabshwnd;
		}

		/**
		 * @brief Resizes the whole tab control
		 * 
		 * @param newsize New size value pair
		 */
		void resize(xy<int> newsize) noexcept;
		/**
		 * @brief Move the tab control to a new position
		 * 
		 * @param newpos New position value pair
		 */
		void move(xy<int> newpos) noexcept;
		/**
		 * @brief Redraws the tab control
		 * 
		 */
		void repaint() const noexcept;
		/**
		 * @brief Calculates the rectangle of tab close button, respective to tabs rectangle
		 * 
		 * @param itemRect Tabs rectangle
		 * @return RECT Calculated rectangle
		 */
		[[nodiscard]] static RECT s_calcCloseButton(RECT itemRect) noexcept;
		/**
		 * @brief Inserts a new tab at a given index to the tab control
		 * 
		 * @param title Tab title
		 * @param index Tab index, Tabs::endpos by default
		 * @return listtype::iterator Iterator of newly inserted tab
		 */
		listtype::iterator insert(std::wstring_view title, const ssize_t index = Tabs::endpos);
		/**
		 * @brief Removes a tab with the specified name
		 * 
		 * @param title Tab name
		 */
		void remove(std::wstring_view title) noexcept;
		/**
		 * @brief Remove a tab at the specified index
		 * 
		 * @param index Tab index, Tabs::endpos by default
		 */
		void remove(const ssize_t index = Tabs::endpos) noexcept;
		/**
		 * @brief Renames a tab at the specified index
		 * 
		 * @param title Tab new title
		 * @param index Tab index, Tabs::endpos by default
		 * @return listtype::iterator Iterator of renamed tabs
		 */
		listtype::iterator rename(std::wstring_view title, const ssize_t index = Tabs::endpos);
		/**
		 * @brief Get tab name
		 * 
		 * @param index Tab index, Tabs::endpos by default
		 * @return std::wstring_view Tab's name
		 */
		[[nodiscard]] std::wstring_view getName(const ssize_t index = Tabs::endpos) const noexcept;
		/**
		 * @brief Selects a tab, makes it active
		 * 
		 * @param index Tab index, Tabs::endpos by default
		 */
		void select(const ssize_t index = Tabs::endpos) noexcept;
		/**
		 * @return std::size_t Number of tab currently present
		 */
		[[nodiscard]] std::size_t size() const noexcept;
		/**
		 * @brief Handles the tab control active tab changes
		 * 
		 */
		void selChange() noexcept;

	};
}
