#pragma once
#include "common.hpp"

namespace pdfv {
	namespace error {
		enum errorcode : std::uint32_t {
			success,
			noerror = success,
			error,
			registerclass,
			registerabout,
			registertab,
			commoncontrols,
			window,

			pdf_success,
			pdf_unknown,
			pdf_file,
			pdf_format,
			pdf_password,
			pdf_security,
			pdf_page,

			max_error
		};

		//
		// 	An array of error messages encoded in UTF-16 (Windows Unicode)
		//
		extern const wchar_t* errMsgs[max_error];

		//
		//	Displays error messages based on error id
		//	Makes handle (passed as second parameter)
		//	the owner of the Message Box
		//
		void Report(errorcode errid, HWND hwnd) noexcept;
		//
		//	Displays error messages based on error id
		//	Message Box owner is the main window
		//
		void Report(errorcode errid) noexcept;
	}
}