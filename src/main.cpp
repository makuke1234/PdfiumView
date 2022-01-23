#include "common.hpp"
#include "mainwindow.hpp"
#include "otherwindow.hpp"

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR cmdLine, int nCmdShow)
{
	DEBUGPRINT("wWinMain(%p, , %p, %d)\n", static_cast<void *>(hInst), static_cast<void *>(cmdLine), nCmdShow);

	int argc;
	auto argv{ pdfv::getArgs(cmdLine, argc) };

	wchar_t fname[MAX_PATH]{};
	if (argv != nullptr && argc > 1)
	{
		::GetFullPathNameW(argv.get()[1], MAX_PATH, fname, nullptr);
	}

	pdfv::OtherWindow otherWnd{ fname };
	if (otherWnd.exists())
	{
		return pdfv::error::success;
	}

	if (!pdfv::window.init(hInst, argc, argv.get())) [[unlikely]]
	{
		pdfv::error::report();
		return pdfv::error::error;
	}

	if (!pdfv::window.run(fname, nCmdShow)) [[unlikely]]
	{
		pdfv::error::report();
		return pdfv::error::error;
	}	

	return pdfv::window.msgLoop();
}
