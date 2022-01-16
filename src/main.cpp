#include "common.hpp"
#include "mainwindow.hpp"
#include "otherwindow.hpp"

#include <shellapi.h>

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int nCmdShow)
{
	using namespace pdfv;

	int argc{};
	wchar_t ** argv{ ::CommandLineToArgvW(::GetCommandLineW(), &argc) };
	
	wchar_t fname[MAX_PATH] = { 0 };
	if (argv != nullptr)
	{
		if (argc > 1)
		{
			::GetFullPathNameW(argv[1], MAX_PATH, fname, nullptr);
		}
	}

	auto otherWnd = OtherWindow(fname);

	if (otherWnd.exists())
	{
		return 0;
	}

	::SetProcessDPIAware();

	INITCOMMONCONTROLSEX iccex{};
	iccex.dwSize = sizeof iccex;
	iccex.dwICC  = ICC_WIN95_CLASSES;

	if (!::InitCommonControlsEx(&iccex))
	{
		error::report(error::commoncontrols);
		return error::error;
	}

	MainWindow::mwnd.m_wcex.style        = CS_HREDRAW | CS_VREDRAW;
	MainWindow::mwnd.m_wcex.lpfnWndProc  = &pdfv::MainWindow::windowProc;
	MainWindow::mwnd.m_wcex.hInstance    = hInst;
	MainWindow::mwnd.m_wcex.hIcon        = ::LoadIconW(hInst, IDI_APPLICATION);
	MainWindow::mwnd.m_wcex.lpszMenuName = MAKEINTRESOURCEW(IDR_MAINMENU);
	MainWindow::mwnd.m_wcex.hIconSm      = ::LoadIconW(hInst, IDI_APPLICATION);

	if (!registerClasses(MainWindow::mwnd.m_wcex))
	{
		error::report(error::registerclass);
		return error::error;
	}

	auto wc = MainWindow::mwnd.m_wcex;

	wc.lpfnWndProc   = &pdfv::TabObject::tabProc;
	wc.lpszClassName = APP_CLASSNAME "Tab";
	wc.lpszMenuName  = nullptr;
	wc.hbrBackground = ::CreateSolidBrush(RGB(255, 255, 255));
	
	if (!registerClasses(wc))
	{
		error::report(error::registertab);
		return error::error;
	}

	wc.lpfnWndProc   = &pdfv::MainWindow::aboutProc;
	wc.lpszClassName = APP_CLASSNAME "About";
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW);
	
	if (!registerClasses(wc))
	{
		error::report(error::registerabout);
		MainWindow::mwnd.m_helpAvailable = false;
	}

	auto hwnd = ::CreateWindowExW(
		0,
		APP_CLASSNAME,
		APP_NAME,
		WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		nullptr,
		nullptr,
		hInst,
		fname
	);
	if (hwnd == nullptr)
	{
		error::report(error::window);
		return error::error;
	}

	// Add "about" to system menu (menu when caption bar is right-clicked)
	if (MainWindow::mwnd.m_helpAvailable) {
		auto hSysMenu = ::GetSystemMenu(hwnd, false);
		::InsertMenuW(hSysMenu, 5, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
		::InsertMenuW(hSysMenu, 6, MF_BYPOSITION, IDM_HELP_ABOUT, L"&About");
	}
	else
	{
		::EnableMenuItem(::GetMenu(hwnd), IDM_HELP_ABOUT, MF_DISABLED);
	}
	auto hAccelerators = ::LoadAcceleratorsW(hInst, MAKEINTRESOURCEW(IDR_ACCELERATOR1));

	::ShowWindow(hwnd, nCmdShow);
	// Good practise
	::UpdateWindow(hwnd);

	MSG msg{};
	BOOL bRet{};
	while ((bRet = ::GetMessageW(&msg, nullptr, 0, 0)) != 0)
	{
		if (bRet == -1) [[unlikely]]
		{
			// Some error happened
			DWORD lastError = ::GetLastError();
			wchar_t errText[256], temp[256];
			switch (lastError)
			{
			case ERROR_INVALID_WINDOW_HANDLE:
			case ERROR_INVALID_ACCEL_HANDLE:
			case ERROR_INVALID_MENU_HANDLE:
			case ERROR_INVALID_ICON_HANDLE:
			case ERROR_INVALID_CURSOR_HANDLE:
				::FormatMessageW(
					FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					nullptr,
					lastError,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					temp,
					256,
					nullptr
				);
				::swprintf_s(errText, L"%s\nError code: " PTRPRINT, temp, lastError);
				break;
			default:
				::swprintf_s(errText, L"Unknown hard error!\nError code: " PTRPRINT, lastError);
			}
			MainWindow::mwnd.message(errText, MB_ICONERROR | MB_OK);
			return lastError;
		}
		else [[likely]]
		{
			if (!::TranslateAcceleratorW(hwnd, hAccelerators, &msg))
			{
				if (!::IsDialogMessageW(hwnd, &msg))
				{
					::TranslateMessage(&msg);
					::DispatchMessageW(&msg);
				}
			}
		}
	}
	
	return int(msg.wParam);
}
