// If in release mode, empty translation unit
#ifdef _DEBUG

#include "debug.hpp"
#include <cstdio>
#include <cstdarg>
#include <cstring>

namespace pdfv::debug
{
	struct PrintData
	{
		bool enabled{ false };
		FILE * file{ nullptr };

		~PrintData() noexcept
		{
			this->enabled = false;
			if (this->file != nullptr)
			{
				std::fclose(this->file);
				this->file = nullptr;
			}
		}

	} printData;
}

int pdfv::debug::printf(const char * format, ...) noexcept
{
	auto innerCommand = [](const char * cmd) noexcept -> int
	{
		// Scan for first non-space character
		for (; *cmd == ' ' && *cmd != '\0'; ++cmd);
		if (std::strncmp(cmd, "open", 4) == 0)
		{
			cmd += 4;
			for (; *cmd == ' ' && *cmd != '\0' && *(cmd + 1) != '"'; ++cmd);
			// try to open debug file
			printData.file = std::fopen(cmd, "w");
			if (printData.file == nullptr) [[unlikely]]
			{
				return 0;
			}
			else [[likely]]
			{
				printData.enabled = true;
				return 1;
			}
		}
		else if (std::strcmp(cmd, "close") == 0)
		{
			if (printData.enabled == false) [[unlikely]]
			{
				return 0;
			}
			else [[likely]]
			{
				printData.enabled = false;
				std::fclose(printData.file);
				printData.file = nullptr;
				return 1;
			}
		}
		else if (std::strcmp(cmd, "enable") == 0)
		{
			if (printData.file != nullptr) [[likely]]
			{
				printData.enabled = true;
				return 1;
			}
			else [[unlikely]]
			{
				return 0;
			}
		}
		else if (std::strcmp(cmd, "disable") == 0)
		{
			printData.enabled = false;
			return 1;
		}
		else
		{
			return 0;
		}
	};

	constexpr const char specialFormat[]{ "\\>" };
	constexpr auto specialFormatLength{ sizeof(specialFormat) / sizeof(char) - 1 };

	// Check for "special format"
	if (std::strncmp(format, specialFormat, specialFormatLength) == 0)
	{
		return innerCommand(format + specialFormatLength);
	}
	else if (printData.enabled)
	{
		va_list ap;
		va_start(ap, format);
		auto ret{ std::vfprintf(printData.file, format, ap) };
		va_end(ap);
		return ret;
	}
	else
	{
		va_list ap;
		va_start(ap, format);
		auto ret{ std::vprintf(format, ap) };
		va_end(ap);
		return ret;
	}
}



#endif
