#pragma once
#include "common.hpp"
#include "tabs.hpp"
#include "opendialog.hpp"

#include <atomic>

namespace pdfv
{
	class OtherWindow;

	class MainWindow
	{
	public:
		static inline const std::wstring defaulttitle{ PRODUCT_NAME };

	private:
		MainWindow() noexcept;

		int m_argc{};
		wchar_t ** m_argv{ nullptr };
		HINSTANCE m_hInst{ nullptr };
		HACCEL m_hAccelerators{ nullptr };

		xy<int> m_usableArea;
		xy<int> m_totalArea;
		xy<int> m_minArea;
		xy<int> m_pos;
		xy<int> m_border;
		int m_menuSize{ 0 };
		HWND m_hwnd{ nullptr }, m_dlg{ nullptr };
		WNDCLASSEXW m_wcex{};
		std::wstring m_title;
		w::GDI<HFONT> m_defaultFont{ nullptr }, m_defaultFontBold{ nullptr };
		bool m_helpAvailable{ true };
		pdfv::Tabs m_tabs;
		pdfv::OpenDialog m_openDialog{ 2048 };

		w::GDI<HBRUSH> m_redBrush{ ::CreateSolidBrush(RGB(237, 28, 36)) };
		w::GDI<HBRUSH> m_brightRedBrush{ ::CreateSolidBrush(RGB(255, 128, 128)) };
		w::GDI<HBRUSH> m_darkRedBrush{ ::CreateSolidBrush(RGB(200, 0, 0)) };
		std::atomic<bool> m_highlighted{ false };
		std::atomic<std::size_t> m_highlightedIdx{ 0 };
		HANDLE m_moveThread{ nullptr };
		bool m_moveKillSwitch{ false }, m_closeButtonDown{ false };

		/**
		 * @brief Tells if mouse cursor intersects with any tabs' close button,
		 * if intersects, the function sets m_highlightedIdx member variable
		 * 
		 * @return true Intersects
		 * @return false Doesn't intersects
		 */
		[[nodiscard]] bool intersectsTabClose() noexcept;

		friend class OtherWindow;
		/**
		 * @brief Shows about dialog box
		 * 
		 */
		void aboutBox() noexcept;
		std::wstring m_aboutText{ DEFAULT_ABOUT_TEXT };

	public:
		//
		//	A singleton instance of the class supported by a private constructor
		//
		static MainWindow mwnd;

		MainWindow(const MainWindow & other) = delete;
		MainWindow(MainWindow && other) noexcept = delete;
		MainWindow & operator=(const MainWindow & other) = delete;
		MainWindow & operator=(MainWindow && other) noexcept = delete;
		~MainWindow() noexcept;

		/**
		 * @brief Initialise window object
		 * 
		 * @param hinst Module handle
		 * @param argc Argument vector length
		 * @param argv Argument vector
		 * @return true Success
		 * @return false Failure
		 */
		[[nodiscard]] bool init(HINSTANCE hinst, int argc, wchar_t ** argv) noexcept;
		/**
		 * @brief Run main window
		 * 
		 * @param fname PDF file name to open
		 * @param nCmdShow Window show flag
		 * @return true Success
		 * @return false Failure
		 */
		[[nodiscard]] bool run(const wchar_t * fname, int nCmdShow) noexcept;
		/**
		 * @brief Message loop
		 */
		int msgLoop() const noexcept;

		/**
		 * @brief Enables/disable main window (unblocks/blocks input)
		 * 
		 * @param enable true by default
		 */
		void enable(bool enable = true) const noexcept;
		/**
		 * @return const WNDCLASSEXW& Current window class extended object
		 */
		[[nodiscard]] constexpr const WNDCLASSEXW & getWinClass() const noexcept
		{
			return this->m_wcex;
		}
		[[nodiscard]] constexpr const HWND & getHwnd() const noexcept
		{
			return this->m_hwnd;
		}
		[[nodiscard]] constexpr HINSTANCE getHinst() const noexcept
		{
			return this->m_wcex.hInstance;
		}
		[[nodiscard]] constexpr const std::wstring & getTitle() const noexcept
		{
			return this->m_title;
		}
		/**
		 * @brief Set new window title
		 * 
		 * @param newTitle New title
		 */
		void setTitle(std::wstring_view newTitle);
		/**
		 * @return const xy<int>& Current border width
		 */
		[[nodiscard]] constexpr const xy<int> & border() const noexcept
		{
			return this->m_border;
		}
		[[nodiscard]] constexpr const HFONT & getDefFont() const noexcept
		{
			return this->m_defaultFont;
		}
		[[nodiscard]] constexpr const HFONT & getDefFontBold() const noexcept
		{
			return this->m_defaultFontBold;
		}
		
		/**
		 * @brief Wrapper function to display messages with this window as the ownder of the messagebox
		 * 
		 * @param message Message to display
		 * @param msgtitle Messagebox title
		 * @param type Messagebox type, MB_OK by default
		 * @return int Messagebox return value
		 */
		int message(LPCWSTR message, LPCWSTR msgtitle, UINT type = MB_OK) const noexcept;
		/**
		 * @brief Wrapper function to display messages with this window as the ownder of the messagebox
		 * 
		 * @param message Message to display, empty string by default
		 * @param type Messagebox type, MB_OK by default
		 * @return int Messagebox return value
		 */
		int message(LPCWSTR message = L"", UINT type = MB_OK) const noexcept;
		
		static constexpr UINT WM_LLMOUSEHOOK  = WM_USER;
		static constexpr UINT WM_BRINGTOFRONT = WM_USER + 1;
		static constexpr UINT WM_TABMOUSEMOVE = WM_USER + 2;
		
	private:
		/**
		 * @brief Win32 API callback function for the main window class
		 * 
		 */
		static LRESULT CALLBACK windowProc(const HWND hwnd, const UINT uMsg, WPARAM wp, LPARAM lp) noexcept;
		
		LRESULT wOnDrawItem(DRAWITEMSTRUCT * dis) noexcept;
		void wOnCommand(WPARAM wp) noexcept;
		void wOnKeydown(WPARAM wp) noexcept;
		void wOnMousewheel(WPARAM wp) noexcept;
		void wOnLButtonDown(WPARAM wp, LPARAM lp) noexcept;
		void wOnLButtonUp(WPARAM wp, LPARAM lp) noexcept;
		void wOnTabMouseMove(WPARAM wp, LPARAM lp) noexcept;
		LRESULT wOnNotify(LPARAM lp) noexcept;
		void wOnMove(LPARAM lp) noexcept;
		void wOnSizing(WPARAM wp, LPARAM lp) noexcept;
		void wOnSize() noexcept;
		void wOnCreate(HWND hwnd, LPARAM lp) noexcept;
		void wOnCopydata(LPARAM lp) noexcept;
		void wOnBringToFront() noexcept;

		/**
		 * @brief Win32 API callback function for Help->About dialog
		 * 
		 */
		static INT_PTR CALLBACK aboutProc(const HWND hwnd, const UINT uMsg, WPARAM wp, LPARAM lp) noexcept;

		/**
		 * @brief Opens PDF file
		 * 
		 * @param file PDF file name
		 */
		void openPdfFile(std::wstring_view file) noexcept;
	};

	constexpr inline auto& window = MainWindow::mwnd;
}
