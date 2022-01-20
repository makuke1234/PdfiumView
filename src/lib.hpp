#pragma once

#include "common.hpp"
#include "hdcbuffer.hpp"

#include <vector>

namespace pdfv
{
	class Pdfium
	{
	private:
		static inline bool s_errorHappened{};
		static inline bool s_libInit{};

		FPDF_DOCUMENT m_fdoc{ nullptr };
		FPDF_PAGE m_fpage{ nullptr };
		std::size_t m_fpagenum{};
		std::size_t m_numPages{};
		
		std::unique_ptr<std::uint8_t> m_buf{ nullptr };

		hdc::Renderer m_optRenderer;

	public:
		Pdfium() noexcept;
		Pdfium(const Pdfium & other) = delete;
		Pdfium(Pdfium && other) noexcept;
		Pdfium & operator=(const Pdfium & other) = delete;
		Pdfium & operator=(Pdfium && other) noexcept;
		~Pdfium() noexcept;

		static void init() noexcept;
		static void free() noexcept;

		//
		//	Returns the last error of the pdfium library
		//
		[[nodiscard]] error::Errorcode getLastError() noexcept;
		//
		//	Loads a PDF file from specified path given as an UTF-8 string
		//	Also loads a given page, which is first page by default
		//	Returns an errorcode
		//
		error::Errorcode pdfLoad(std::string_view path, std::size_t page = 1);
		//
		//	Loads a PDF file from specified path given as an UTF-16 (Unicode) string
		//	Also loads a given page, which is first page by default
		//	Returns an errorcode
		//
		error::Errorcode pdfLoad(const std::wstring & path, std::size_t page = 1);
		//
		//	Loads a PDF file from specified path given as a std::vector of bytes.
		//	Also loads a given page, which is first page by default
		//	Returns an errorcode
		//
		error::Errorcode pdfLoad(const u8 * data, std::size_t length, std::size_t page = 1);
		//
		//
		//
		error::Errorcode pdfLoad(u8 * && data, std::size_t length, std::size_t page = 1) noexcept;
		//
		//	Unloads (closes) the currently loaded PDF if opened any
		//
		void pdfUnload() noexcept;

		//
		//	Loads a given page from the currently loaded PDF file if any
		//	Return an errorcode
		//
		error::Errorcode pageLoad(std::size_t page) noexcept;
		//
		//	Unloads (closes) the currently loaded page from the currently
		//	loaded PDF file if opened any
		//
		void pageUnload() noexcept;
		//
		//	Renders the current page of the PDF document to the specified
		//	position on the device context with specified size
		//
		error::Errorcode pageRender(HDC dc, pdfv::xy<int> pos, pdfv::xy<int> size);

		//
		//	Returns the page count of the currently opened PDF document
		//	Returns 0 if none is open
		//
		[[nodiscard]] constexpr std::size_t pageGetCount() const noexcept
		{
			return this->m_numPages;
		}
		//
		//	Returns the page number of the currently open page
		//	Returns 0 if none is open
		//
		[[nodiscard]] constexpr std::size_t pageGetNum() const noexcept
		{
			return this->m_fpagenum;
		}
		//
		//	Returns whether the PDF document is loaded or not
		//
		[[nodiscard]] constexpr bool pdfExists() const noexcept
		{
			return this->m_fdoc != nullptr;
		}
	};
}
