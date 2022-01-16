#pragma once
#include "common.hpp"
#include <memory>

namespace pdfv
{
	class OpenDialog
	{
	private:
		std::unique_ptr<wchar_t> m_lastName{ nullptr };
		std::unique_ptr<wchar_t> m_currentName{ nullptr };
		std::size_t m_bufSize{ 0 };

		static inline const wchar_t * s_cDefaultOpenFilter{
			L"PDF documents (*.pdf)\0*.pdf\0All files (*.*)\0*.*\0"
		};
		static inline const int s_cDefaultOpenFilterIndex{ 1 };
		static inline const wchar_t * s_cDefaultOpenTitle{
			L"Open PDF document..."
		};

	public:
		OpenDialog() noexcept = delete;
		OpenDialog(std::size_t bufsize);
		OpenDialog(std::wstring_view defaultName, pdfv::ssize_t bufsize = -1);
		OpenDialog(wchar_t * && moveDefaultName, pdfv::ssize_t bufsize = -1);
		OpenDialog(const OpenDialog & other);
		OpenDialog(OpenDialog && other) noexcept;
		OpenDialog & operator=(const OpenDialog & other);
		OpenDialog & operator=(OpenDialog && other) noexcept;
		~OpenDialog() noexcept = default;

		[[nodiscard]] bool open(HWND hwnd, std::wstring & output);
		bool updateName(std::wstring_view newfilename) noexcept;
	};
}