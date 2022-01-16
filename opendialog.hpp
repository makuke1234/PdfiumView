#pragma once
#include "common.hpp"
#include <memory>

namespace pdfv {
	class opendialog {
	private:
		std::unique_ptr<wchar_t> m_lastName{ nullptr };
		std::unique_ptr<wchar_t> m_currentName{ nullptr };
		std::size_t m_bufSize{};

		static inline const wchar_t* s_cDefaultOpenFilter{
			L"PDF documents (*.pdf)\0*.pdf\0All files (*.*)\0*.*\0"
		};
		static inline const int s_cDefaultOpenFilterIndex{ 1 };
		static inline const wchar_t* s_cDefaultOpenTitle{
			L"Open PDF document..."
		};

	public:
		opendialog() noexcept = delete;
		opendialog(std::size_t bufsize);
		opendialog(std::wstring_view defaultName, pdfv::ssize_t bufsize = -1);
		opendialog(wchar_t*&& moveDefaultName, pdfv::ssize_t bufsize = -1);
		opendialog(opendialog const& other);
		opendialog(opendialog&& other) noexcept;
		opendialog& operator=(opendialog const& other);
		opendialog& operator=(opendialog&& other) noexcept;
		~opendialog() noexcept = default;

		[[nodiscard]] bool Open(HWND hwnd, std::wstring& output);
		bool UpdateName(std::wstring_view newfilename) noexcept;
	};
}