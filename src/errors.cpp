#include "errors.hpp"
#include "mainwindow.hpp"

const wchar_t * pdfv::error::errMsgs[pdfv::error::max_error]
{
	L"No error.",
	L"Program reported an error.",
	L"Error registering application class!",
	L"Error registering tab control class!",
	L"Error initializing common controls (a.k.a. XP-styles)!",
	L"Error creating window!",
	
	L"No PDF error.",
	L"Unknown error occurred while opening PDF!",
	L"PDF file could not be located!",
	L"PDF file is in wrong format!",
	L"PDF file required a password!",
	L"PDF file could not be accessed!",
	L"Selected page in the PDF could not be opened!"
};

void pdfv::error::report(pdfv::error::Errorcode errid, HWND hwnd) noexcept
{
	DEBUGPRINT("pdfv::error::report(%d, %p)\n", errid, static_cast<void *>(hwnd));
	
	static const wchar_t * emptyTitle{ L"" };

	assert((errid >= success) && (errid < max_error));
	const wchar_t * temp{ emptyTitle };
	wchar_t tempabstract[2048];
	if (hwnd != nullptr) [[likely]]
	{
		if (::GetWindowTextW(hwnd, tempabstract, 2048) > 0) [[likely]]
		{
			temp = tempabstract;
		}
	}
	::MessageBoxW(hwnd, errMsgs[errid], temp, MB_ICONERROR | MB_OK);
}
void pdfv::error::report(pdfv::error::Errorcode errid) noexcept
{
	pdfv::error::report(errid, MainWindow::mwnd.getHwnd());
}

pdfv::error::Errorcode pdfv::error::lastErr = pdfv::error::Errorcode::success;

void pdfv::error::report(HWND hwnd) noexcept
{
	report(lastErr, hwnd);
}

void pdfv::error::report() noexcept
{
	report(lastErr);
}
