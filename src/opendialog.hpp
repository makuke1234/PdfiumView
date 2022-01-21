#pragma once
#include "common.hpp"
#include <memory>

namespace pdfv
{
	class OpenDialog
	{
	private:
		std::unique_ptr<wchar_t> m_lastName{ nullptr };
		std::size_t m_bufSize{ 0 };

		static constexpr const wchar_t * s_cDefaultOpenFilter{ L"PDF documents (*.pdf)\0*.pdf\0All files (*.*)\0*.*\0" };
		static constexpr int s_cDefaultOpenFilterIndex{ 1 };
		static constexpr const wchar_t * s_cDefaultOpenTitle{ L"Open PDF document..." };

	public:
		OpenDialog() noexcept = delete;
		OpenDialog(std::size_t bufsize);
		/**
		 * @brief Construct a new Open Dialog object with given default filename and buffer size
		 * 
		 * @param defaultName Default file name
		 * @param bufsize Buffer size, -1 by default, meaning that the buffer size would be the length of default file name
		 */
		OpenDialog(std::wstring_view defaultName, pdfv::ssize_t bufsize = -1);
		/**
		 * @brief Construct a new Open Dialog object with given default filename and buffer size
		 * 
		 * @param defaultName Default file name, will be consumed
		 * @param bufsize Buffer size, -1 by default, meaning that the buffer size would be the length of default file name
		 */
		OpenDialog(wchar_t * && moveDefaultName, pdfv::ssize_t bufsize = -1);
		OpenDialog(const OpenDialog & other);
		OpenDialog(OpenDialog && other) noexcept;
		OpenDialog & operator=(const OpenDialog & other);
		OpenDialog & operator=(OpenDialog && other) noexcept;
		~OpenDialog() noexcept = default;

		/**
		 * @brief Opens dialog file open dialog
		 * 
		 * @param hwnd Window handle of the dialog's owner
		 * @param output Output string from dialog
		 * @return true Retrieving output was successful
		 * @return false 
		 */
		[[nodiscard]] bool open(HWND hwnd, std::wstring & output);
		/**
		 * @brief Updates default file name
		 * 
		 * @param newfilename New default file name for file open dialog
		 * @return true Success
		 * @return false Failure
		 */
		bool updateName(std::wstring_view newfilename) noexcept;
	};
}