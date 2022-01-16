#include "errors.hpp"
#include "main_window.hpp"

const wchar_t* pdfv::error::errMsgs[pdfv::error::max_error] {
	L"No error.",
	L"Program reported an error.",
	L"Error registering application class!",
	L"Error registering about box class!",
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

void pdfv::error::Report(pdfv::error::errorcode errid, HWND hwnd) noexcept
{
	static const wchar_t *emptyTitle = L"";

	assert((errid >= success) && (errid < max_error));
	wchar_t *temp = const_cast<wchar_t*>(emptyTitle), tempabstract[2048];
	if (hwnd != nullptr) {
		::GetWindowTextW(hwnd, tempabstract, 2048);
		temp = tempabstract;
	}
	::MessageBoxW(hwnd, errMsgs[errid], temp, MB_ICONERROR | MB_OK);
}
void pdfv::error::Report(pdfv::error::errorcode errid) noexcept
{
	pdfv::error::Report(errid, main_window::mwnd.GetHWND());
}
