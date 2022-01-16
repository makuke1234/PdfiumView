#pragma once
#include "common.hpp"
#include "tabs.hpp"
#include "opendialog.hpp"

namespace pdfv {
	class main_window {
	private:
		main_window() noexcept;
		xy<int> m_usableArea;
		xy<int> m_totalArea;
		xy<int> m_minArea;
		xy<int> m_pos;
		xy<int> m_border;
		int m_menuSize{};
		HWND m_hwnd{};
		WNDCLASSEXW m_wcex{};
		std::wstring m_title;
		HFONT m_defaultFont{};
		bool m_helpAvailable{ true };
		pdfv::tabs m_tabs;
		pdfv::opendialog m_openDialog{ 2048 };

		friend int WINAPI ::wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int);
		static void AboutBox() noexcept;
		static inline const int s_cAboutBoxSizeX{ 500 };
		static inline const int s_cAboutBoxSizeY{ 400 };

	public:
		//
		//	A singleton instance of the class supported by a private constructor
		//
		static main_window mwnd;

		main_window(main_window const& other) = delete;
		main_window(main_window&& other) noexcept = delete;
		main_window& operator=(main_window const& other) = delete;
		main_window& operator=(main_window&& other) noexcept = delete;
		~main_window() noexcept;

		//
		//	Enables/disables main window (unblocks/block input)
		//
		void Enable(bool enable = true) const noexcept;
		//
		//	Returns const reference to currently used WNDCLASSEXW structure
		//
		[[nodiscard]] constexpr WNDCLASSEXW const& GetWindowClass() const noexcept
		{
			return m_wcex;
		}
		[[nodiscard]] constexpr HWND const& GetHWND() const noexcept
		{
			return m_hwnd;
		}
		[[nodiscard]] constexpr HINSTANCE GetHINST() const noexcept
		{
			return m_wcex.hInstance;
		}
		[[nodiscard]] constexpr std::wstring const& GetTitle() const noexcept
		{
			return m_title;
		}
		void SetTitle(std::wstring_view newTitle);
		//
		//	Returns const reference to current border width
		//
		[[nodiscard]] constexpr xy<int> const& Border() const noexcept
		{
			return m_border;
		}
		[[nodiscard]] constexpr HFONT const& GetDefaultFont() const noexcept
		{
			return m_defaultFont;
		}
		
		//
		//	A wrapper function to display messages with this window as the
		//	"owner" of the Message Box
		//
		int Message(std::wstring_view message, std::wstring_view msgtitle, UINT type = MB_OK) const noexcept;
		//
		//	A wrapper function to display messages with this window as the
		//	"owner" of the Message Box
		//
		int Message(std::wstring_view message = L"", UINT type = MB_OK) const noexcept;
		
	private:
		static inline constexpr UINT WM_LLMOUSEHOOK = WM_USER + 1;
		static inline constexpr UINT WM_BRINGTOFRONT = WM_USER + 2;
		
		//
		//	The Windows API callback function for the main window "class"
		//
		static LRESULT CALLBACK WindowProc(const HWND hwnd, const UINT uMsg, WPARAM wp, LPARAM lp) noexcept;
		//
		//	The Windows API callback function for the Help->About "class"
		//
		static LRESULT CALLBACK AboutProc(const HWND hwnd, const UINT uMsg, WPARAM wp, LPARAM lp) noexcept;

		//
		//	Handles the opening of PDF documents
		//
		void OpenPdfFile(std::wstring_view file) noexcept;
	};

	constexpr inline auto& window = main_window::mwnd;
}
