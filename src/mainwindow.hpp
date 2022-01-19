#pragma once
#include "common.hpp"
#include "tabs.hpp"
#include "opendialog.hpp"

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
		int m_menuSize{};
		HWND m_hwnd{ nullptr };
		WNDCLASSEXW m_wcex{};
		std::wstring m_title;
		HFONT m_defaultFont{ nullptr };
		bool m_helpAvailable{ true };
		pdfv::Tabs m_tabs;
		pdfv::OpenDialog m_openDialog{ 2048 };

		HBRUSH m_redBrush{ ::CreateSolidBrush(RGB(237, 28, 36)) };
		HBRUSH m_brightRedBrush{ ::CreateSolidBrush(RGB(255, 128, 128)) };
		bool m_highlighted{ false };

		friend class OtherWindow;
		static void aboutBox() noexcept;
		static inline const int s_cAboutBoxSizeX{ 500 };
		static inline const int s_cAboutBoxSizeY{ 400 };

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
		[[nodiscard]] constexpr const HFONT & getDefaultFont() const noexcept
		{
			return this->m_defaultFont;
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
		static LRESULT CALLBACK aboutProc(const HWND hwnd, const UINT uMsg, WPARAM wp, LPARAM lp) noexcept;

		//
		//	Handles the opening of PDF documents
		//
		void openPdfFile(std::wstring_view file) noexcept;
	};

	constexpr inline auto& window = MainWindow::mwnd;
}
