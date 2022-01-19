#pragma once

// If in release mode, empty header file
#ifdef _DEBUG

namespace pdfv::debug
{
	int printf(const char * format, ...) noexcept;
}


#endif
