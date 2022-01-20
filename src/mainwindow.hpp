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

		[[nodiscard]] bool intersectsTabClose() noexcept;

		friend class OtherWindow;
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

		[[nodiscard]] bool init(HINSTANCE hinst, int argc, wchar_t ** argv) noexcept;
		[[nodiscard]] bool run(const wchar_t * fname, int nCmdShow) noexcept;
		int msgLoop() const noexcept;

		//
		//	Enables/disables main window (unblocks/block input)
		//
		void enable(bool enable = true) const noexcept;
		//
		//	Returns const reference to currently used WNDCLASSEXW structure
		//
		[[nodiscard]] constexpr const WNDCLASSEXW & getWindowClass() const noexcept
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
		void setTitle(std::wstring_view newTitle);
		//
		//	Returns const reference to current border width
		//
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
		
		//
		//	A wrapper function to display messages with this window as the
		//	"owner" of the Message Box
		//
		int message(LPCWSTR message, LPCWSTR msgtitle, UINT type = MB_OK) const noexcept;
		//
		//	A wrapper function to display messages with this window as the
		//	"owner" of the Message Box
		//
		int message(LPCWSTR message = L"", UINT type = MB_OK) const noexcept;
		
		static constexpr UINT WM_LLMOUSEHOOK  = WM_USER;
		static constexpr UINT WM_BRINGTOFRONT = WM_USER + 1;
		static constexpr UINT WM_TABMOUSEMOVE = WM_USER + 2;
		
	private:
		//
		//	The Windows API callback function for the main window "class"
		//
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

		//
		//	The Windows API callback function for the Help->About "class"
		//
		static INT_PTR CALLBACK aboutProc(const HWND hwnd, const UINT uMsg, WPARAM wp, LPARAM lp) noexcept;

		//
		//	Handles the opening of PDF documents
		//
		void openPdfFile(std::wstring_view file) noexcept;
	};

	constexpr inline auto& window = MainWindow::mwnd;
}
