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
		float zoom{ 1.0f };

		TabObject() noexcept = delete;
		TabObject(std::wstring_view v1, pdfv::Pdfium && v2 = pdfv::Pdfium());
		TabObject(std::wstring && v1, pdfv::Pdfium && v2 = pdfv::Pdfium());
		TabObject(const TabObject & other) = delete;
		TabObject(TabObject && other) noexcept = default;
		TabObject & operator=(const TabObject & other) = delete;
		TabObject & operator=(TabObject && other) noexcept = default;
		~TabObject() noexcept = default;
		
	private:

		int yMaxScroll{};
		int yMinScroll{};
		int page{};

		friend class pdfv::Tabs;
		friend class pdfv::MainWindow;

	};

	class Tabs
	{
	public:
		static constexpr LPCWSTR c_className{ APP_CLASSNAME "TabsCanvas" };

		static constexpr UINT WM_ZOOM     { WM_APP };
		static constexpr UINT WM_ZOOMRESET{ WM_APP + 1 };

		static constexpr ssize_t endpos{ -1 };
		static inline const std::wstring padding{ L"      " };
		static inline const std::wstring defaulttitle{ L"unopened" };
		static inline const std::wstring defaulttitlepadded{ defaulttitle + padding };

	private:
		HWND m_parent{ nullptr }, m_tabshwnd{ nullptr }, m_canvashwnd{ nullptr };
		xy<int> m_pos;
		xy<int> m_size;
		xy<int> m_offset{ 0, dip(23, dpi.y) };

		using ListType = std::vector<pdfv::TabObject>;

		ListType m_tabs;
		ssize_t m_tabindex{ 0 };

		/**
		 * @brief Return pointer to current tab, nullptr, if none is open
		 * 
		 * @return pdfv::TabObject*
		 */
		[[nodiscard]] pdfv::TabObject * curTab() noexcept;
		/**
		 * @brief Return pointer to current tab, nullptr, if none is open
		 * 
		 * @return const pdfv::TabObject* 
		 */
		[[nodiscard]] const pdfv::TabObject * curTab() const noexcept;

		friend class pdfv::MainWindow;

		static constexpr xy<int> s_cCloseButtonSz{ 20, 20 };

		LRESULT tabsCanvasProc(UINT msg, WPARAM wp, LPARAM lp);

		void updatePageCounter() const noexcept;
		void updateZoom() const noexcept;

	public:
		Tabs() noexcept = default;
		/**
		 * @brief Initialises a tab by using parent's window handle and module handle
		 * 
		 * @param hwnd Parent's window handle
		 * @param hInst Module instance handle
		 */
		Tabs(HWND hwnd, HINSTANCE hInst, RECT size) noexcept;
		Tabs(const Tabs & other) = delete;
		Tabs(Tabs && other) noexcept;
		Tabs & operator=(const Tabs & other) = delete;
		Tabs & operator=(Tabs && other) noexcept;
		~Tabs() noexcept;

		static LRESULT CALLBACK tabsCanvasProcHub(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

		/**
		 * @return constexpr const HWND& Tab system window handle
		 */
		[[nodiscard]] constexpr HWND getTabsHandle() const noexcept
		{
			return this->m_tabshwnd;
		}
		[[nodiscard]] constexpr HWND getCanvasHandle() const noexcept
		{
			return this->m_canvashwnd;
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
		void redrawTabs() noexcept;
		/**
		 * @brief Redraws the tab canvas
		 * 
		 */
		void redrawCanvas() const noexcept;
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
		ListType::iterator insert(std::wstring_view title, const ssize_t index = Tabs::endpos);
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
		void remove(ssize_t index = Tabs::endpos) noexcept;
		/**
		 * @brief Renames a tab at the specified index
		 * 
		 * @param title Tab new title
		 * @param index Tab index, Tabs::endpos by default
		 * @return listtype::iterator Iterator of renamed tabs
		 */
		ListType::iterator rename(std::wstring_view title, const ssize_t index = Tabs::endpos);
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

		void updateScrollbar() noexcept;

	};
}
