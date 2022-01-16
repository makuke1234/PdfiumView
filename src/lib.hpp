#pragma once
#include "common.hpp"

#include <vector>

namespace pdfv {
	class lib {
	private:
		static inline bool s_errorHappened{};
		static inline bool s_libInit{};

		FPDF_DOCUMENT m_fdoc{};
		FPDF_PAGE m_fpage{};
		std::size_t m_fpagenum{};
		std::size_t m_numPages{};
		
		std::unique_ptr<std::uint8_t> m_buf{};

	public:
		lib() noexcept;
		lib(lib const& other) = delete;
		lib(lib&& other) noexcept;
		lib& operator=(lib const& other) = delete;
		lib& operator=(lib&& other) noexcept;
		~lib() noexcept;

		static void Init() noexcept;
		static void Destroy() noexcept;

		//
		//	Returns the last error of the pdfium library
		//
		[[nodiscard]] error::errorcode GetLastError() noexcept;
		//
		//	Loads a PDF file from specified path given as an UTF-8 string
		//	Also loads a given page, which is first page by default
		//	Returns an errorcode
		//
		error::errorcode PdfLoad(std::string_view path, std::size_t page = 1) noexcept;
		//
		//	Loads a PDF file from specified path given as an UTF-16 (Unicode) string
		//	Also loads a given page, which is first page by default
		//	Returns an errorcode
		//
		error::errorcode PdfLoad(std::wstring_view path, std::size_t page = 1);
		//
		//	Loads a PDF file from specified path given as a std::vector of bytes.
		//	Also loads a given page, which is first page by default
		//	Returns an errorcode
		//
		error::errorcode PdfLoad(std::vector<std::uint8_t> const& data, std::size_t page = 1) noexcept;
		//
		//	Unloads (closes) the currently loaded PDF if opened any
		//
		void PdfUnload() noexcept;

		//
		//	Loads a given page from the currently loaded PDF file if any
		//	Return an errorcode
		//
		error::errorcode PageLoad(std::size_t page) noexcept;
		//
		//	Unloads (closes) the currently loaded page from the currently
		//	loaded PDF file if opened any
		//
		void PageUnload() noexcept;
		//
		//	Renders the current page of the PDF document to the specified
		//	position on the device context with specified size
		//
		error::errorcode RenderPage(HDC deviceContext, pdfv::xy<int> pos, pdfv::xy<int> size) const noexcept;

		//
		//	Returns the page count of the currently opened PDF document
		//	Returns 0 if none is open
		//
		[[nodiscard]] constexpr std::size_t GetPageCount() const noexcept
		{
			return m_numPages;
		}
		//
		//	Returns the page number of the currently open page
		//	Returns 0 if none is open
		//
		[[nodiscard]] constexpr std::size_t GetPageNum() const noexcept
		{
			return m_fpagenum;
		}
		//
		//	Returns whether the PDF document is loaded or not
		//
		[[nodiscard]] constexpr bool PdfExists() const noexcept
		{
			return m_fdoc != nullptr;
		}
	};
}
