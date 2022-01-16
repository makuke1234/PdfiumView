#include "common.hpp"
#include "main_window.hpp"

#include <shellapi.h>

int WINAPI wWinMain(
	HINSTANCE hInst,
	[[maybe_unused]] HINSTANCE hPrevInst,
	[[maybe_unused]] LPWSTR lpCmdArgs,
	int nCmdShow
)
{
	using namespace pdfv;

	int argc{};
	wchar_t** argv{ ::CommandLineToArgvW(::GetCommandLineW(), &argc) };
	
	std::unique_ptr<wchar_t> fname{ nullptr };
	if (argv != nullptr) {
		if (argc > 1) {
			fname.reset(new wchar_t[2048]);
			::GetFullPathNameW(argv[1], 2048, fname.get(), nullptr);
		}
	}


	HANDLE mutex{};
	{
		mutex = ::CreateMutexW(nullptr, false, APP_CLASSNAME);
		if (::GetLastError() == ERROR_ALREADY_EXISTS) {
			::ReleaseMutex(mutex);
			auto instance = ::FindWindow(APP_CLASSNAME, nullptr);
			if (instance != nullptr) {
				COPYDATASTRUCT cds{};
				cds.dwData = 1;
				cds.cbData = sizeof(wchar_t) * 2048;
				cds.lpData = fname.get();

				::SendMessageW(
					instance,
					WM_COPYDATA,
					0,
					reinterpret_cast<LPARAM>(&cds)
				);
				::SendMessageW(
					instance,
					pdfv::main_window::WM_BRINGTOFRONT,
					0,
					0
				);
			}
			
			return 0;
		}
	}

	::SetProcessDPIAware();

	INITCOMMONCONTROLSEX iccex{};
	iccex.dwSize = sizeof iccex;
	iccex.dwICC  = ICC_WIN95_CLASSES;

	if (!::InitCommonControlsEx(&iccex)) {
		error::Report(error::commoncontrols);
		return error::error;
	}

	main_window::mwnd.m_wcex.style        = CS_HREDRAW | CS_VREDRAW;
	main_window::mwnd.m_wcex.lpfnWndProc  = &pdfv::main_window::WindowProc;
	main_window::mwnd.m_wcex.hInstance    = hInst;
	main_window::mwnd.m_wcex.hIcon        = ::LoadIconW(hInst, IDI_APPLICATION);
	main_window::mwnd.m_wcex.lpszMenuName = MAKEINTRESOURCEW(IDR_MAINMENU);
	main_window::mwnd.m_wcex.hIconSm      = ::LoadIconW(hInst, IDI_APPLICATION);

	if (!RegisterClasses(main_window::mwnd.m_wcex)) {
		error::Report(error::registerclass);
		return error::error;
	}

	auto wc = main_window::mwnd.m_wcex;

	wc.lpfnWndProc   = &pdfv::tabobject::TabProc;
	wc.lpszClassName = APP_CLASSNAME "Tab";
	wc.lpszMenuName  = nullptr;
	wc.hbrBackground = ::CreateSolidBrush(RGB(255, 255, 255));
	
	if (!RegisterClasses(wc)) {
		error::Report(error::registertab);
		return error::error;
	}

	wc.lpfnWndProc   = &pdfv::main_window::AboutProc;
	wc.lpszClassName = APP_CLASSNAME "About";
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW);
	
	if (!RegisterClasses(wc)) {
		error::Report(error::registerabout);
		main_window::mwnd.m_helpAvailable = false;
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
		fname.get()
	);
	if (hwnd == nullptr) {
		error::Report(error::window);
		return error::error;
	}

	// Add "about" to system menu (menu when caption bar is right-clicked)
	if (main_window::mwnd.m_helpAvailable) {
		auto hSysMenu = ::GetSystemMenu(hwnd, false);
		::InsertMenuW(hSysMenu, 5, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
		::InsertMenuW(hSysMenu, 6, MF_BYPOSITION, IDM_HELP_ABOUT, L"&About");
	}
	else
		::EnableMenuItem(::GetMenu(hwnd), IDM_HELP_ABOUT, MF_DISABLED);

	auto hAccelerators = ::LoadAcceleratorsW(hInst, MAKEINTRESOURCEW(IDR_ACCELERATOR1));

	::ShowWindow(hwnd, nCmdShow);
	// Good practise
	::UpdateWindow(hwnd);

	MSG msg{};
	WINBOOL bRet{};
	while ((bRet = ::GetMessageW(&msg, nullptr, 0, 0)) != 0) {
		if (bRet == -1) [[unlikely]] {
			// Some error happened
			DWORD lastError = ::GetLastError();
			wchar_t errText[256], temp[256];
			switch (lastError) {
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
			main_window::mwnd.Message(errText, MB_ICONERROR | MB_OK);
			return lastError;
		}
		else [[likely]] {
			if (!::TranslateAcceleratorW(hwnd, hAccelerators, &msg)) {
				if (!::IsDialogMessageW(hwnd, &msg)) {
					::TranslateMessage(&msg);
					::DispatchMessageW(&msg);
				}
			}
		}
	}
	
	if (mutex != nullptr)
		::ReleaseMutex(mutex);

	return int(msg.wParam);
}
