#pragma once

// If in release mode, empty header file
#ifdef _DEBUG

namespace pdfv::debug
{
	/**
	 * @brief Debug printing function like std::printf, also accepts special commands as format.
	 * Commands:
	 * "open [filename]" -> open file
	 * "close" -> close file
	 * 
	 * @param format Printing format
	 * @param ... Variadic arguments
	 * @return int Number of characters printed in normal mode, on special commands:
	 * 0 -> error; 1 -> success
	 */
	int printf(const char * format, ...) noexcept;
}


#endif
