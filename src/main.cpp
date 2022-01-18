#include "common.hpp"
#include "mainwindow.hpp"
#include "otherwindow.hpp"

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR cmdLine, int nCmdShow)
{
	DEBUGPRINT("wWinMain(%p, , %p, %d)\n", hInst, cmdLine, nCmdShow);

	int argc;
	auto argv = pdfv::getArgs(cmdLine, argc);

	wchar_t fname[MAX_PATH]{};
	if (argv != nullptr && argc > 1)
	{
		::GetFullPathNameW(argv.get()[1], MAX_PATH, fname, nullptr);
	}

	auto otherWnd = pdfv::OtherWindow(fname);
	if (otherWnd.exists())
	{
		return pdfv::error::success;
	}

	if (!pdfv::MainWindow::mwnd.init(hInst, argc, argv.get()))
	{
		pdfv::error::report();
		return pdfv::error::error;
	}

	if (!pdfv::MainWindow::mwnd.run(fname, nCmdShow))
	{
		pdfv::error::report();
		return pdfv::error::error;
	}	

	return pdfv::MainWindow::mwnd.msgLoop();
}
