#pragma once
#include "common.hpp"

namespace pdfv
{
	class MainWindow;

	namespace error
	{
		enum Errorcode : int
		{
			success,
			noerror = success,
			error,
			registerclass,
			registertabscanvas,
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

		/**
		 * @brief An array of error messages encoded in UTF-16 (Windows Unicode)
		 * 
		 */
		extern const wchar_t * errMsgs[max_error];

		/**
		 * @brief Displays error messages
		 * 
		 * @param errid Error message id
		 * @param hwnd Owner of the messagebox
		 */
		void report(Errorcode errid, HWND hwnd) noexcept;
		/**
		 * @brief Displays error messages, messagebox owner is the main window
		 * 
		 * @param errid Error message id
		 * @param win Window const-reference
		 */
		void report(Errorcode errid, const MainWindow & win) noexcept;

		extern Errorcode lastErr;

		/**
		 * @brief Displays error messages based on last error id
		 * Messagebox owner is the main window
		 * 
		 * @param win Window const-reference
		 */
		void report(const MainWindow & win) noexcept;

		/**
		 * @brief Displays error messages based on last error id
		 * @param hwnd Owner of the messagebox
		 * 
		 */
		void report(HWND hwnd) noexcept;
	}
}
