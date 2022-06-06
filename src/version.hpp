#pragma once

#define APP_CLASSNAME  L"PdfiumViewCpp"
#define VERSION_STRING L"0.8b pre2"
#define PRODUCT_NAME   L"PdfiumView"
#define APP_NAME PRODUCT_NAME L" v" VERSION_STRING

#define PRODUCT_GITHUB_PAGE L"https://github.com/makuke1234/PdfiumView"

#define DEFAULT_ABOUT_TEXT \
	PRODUCT_NAME " " VERSION_STRING "\n\n" \
	"A light-weight PDF viewer for Windows. Built in C++ using Win32 API and PDFium library to display PDFs.\n\n" \
	"Supports \"advanced\" tabulation:\n" \
	"Ctrl+Tab\t    \t->  Tabulate forwards\n" \
	"Ctrl+Shift+Tab\t->  Tabulate backwards\n\n" \
	"Planned features:\n" \
	" * Zooming capability\n" \
	" * Search capability\n" \
	" * Ability to open hyperlinks/websites\n\n" \
	"The project is open for any feature requests. Post them under the Issues tab in GitHub."

#define VERSION_SEQUENCE      0,8
#define VERSION_SEQUENCE_STR "0.8"

#define DEFAULT_OPEN_FILTER L""
#define DEFAULT_OPEN_TITLE  L""
