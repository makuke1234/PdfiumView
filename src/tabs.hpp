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
		TabObject(HWND tabshwnd, HINSTANCE hinst, RECT r) noexcept;
		TabObject(HWND tabshwnd, HINSTANCE hInst, RECT r, std::wstring_view V1, pdfv::Pdfium && V2 = pdfv::Pdfium());
		TabObject(HWND tabshwnd, HINSTANCE hInst, RECT r, std::wstring && V1, pdfv::Pdfium && V2 = pdfv::Pdfium()) noexcept;
		TabObject(const TabObject & other) = delete;
		TabObject(TabObject && other) noexcept;
		TabObject & operator=(const TabObject & other) = delete;
		TabObject & operator=(TabObject && other) noexcept;
		~TabObject() noexcept;
		
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
		void updatePDF() noexcept;

	private:
		HWND parent{ nullptr }, tabhandle{ nullptr };
		xy<int> size;

		int yMaxScroll{};
		int yMinScroll{};
		int page{};

		friend class pdfv::Tabs;
		friend class pdfv::MainWindow;

		/**
		 * @brief "Hub" to tabs' window procedures
		 * 
		 */
		static LRESULT CALLBACK tabObjectProcHub(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp);
		/**
		 * @brief "Real" tab's window procedure
		 * 
		 */
		LRESULT tabObjectProc(UINT uMsg, WPARAM wp, LPARAM lp);

		void updateScrollbar() noexcept;
	};

	class Tabs
	{
	public:
		static constexpr ssize_t endpos{ -1 };
		static inline const std::wstring padding{ L"      " };
		static inline const std::wstring defaulttitle{ L"unopened" };
		static inline const std::wstring defaulttitlepadded{ defaulttitle + padding };

	private:
		HWND m_parent{ nullptr }, m_tabshwnd{ nullptr };
		xy<int> m_pos;
		xy<int> m_size;
		xy<int> m_offset;

		using ListType = std::vector<std::unique_ptr<pdfv::TabObject>>;

		ListType m_tabs;
		ssize_t m_tabindex{ 0 };

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
		Tabs(HWND hwnd, HINSTANCE hInst, RECT size) noexcept;
		Tabs(const Tabs & other) = delete;
		Tabs(Tabs && other) noexcept;
		Tabs & operator=(const Tabs & other) = delete;
		Tabs & operator=(Tabs && other) noexcept;
		~Tabs() noexcept;

		/**
		 * @return constexpr const HWND& Tab system window handle
		 */
		[[nodiscard]] constexpr HWND getHandle() const noexcept
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

	};
}
