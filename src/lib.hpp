#pragma once

#include "common.hpp"
#include "hdcbuffer.hpp"

#include <vector>

namespace pdfv
{
	class Pdfium
	{
	private:
		static inline bool s_errorHappened{ false };
		static inline bool s_libInit{ false };

		FPDF_DOCUMENT m_fdoc{ nullptr };
		FPDF_PAGE m_fpage{ nullptr };
		std::size_t m_fpagenum{ 0 };
		std::size_t m_numPages{ 0 };
		
		std::unique_ptr<u8> m_buf{ nullptr };

		hdc::Renderer m_optRenderer;

	public:
		Pdfium() noexcept;
		Pdfium(const Pdfium & other) = delete;
		Pdfium(Pdfium && other) noexcept;
		Pdfium & operator=(const Pdfium & other) = delete;
		Pdfium & operator=(Pdfium && other) noexcept;
		~Pdfium() noexcept;

		/**
		 * @brief Initalise Pdfium library components
		 * 
		 */
		static void init() noexcept;
		/**
		 * @brief Free Pdfium library components
		 * 
		 */
		static void free() noexcept;

		/**
		 * @return error::Errorcode The last error of the Pdfium library
		 */
		[[nodiscard]] static error::Errorcode getLastError() noexcept;
		/**
		 * @brief Loads PDF file from path given as UTF-8 string, loads given page, first page by default
		 * 
		 * @param path UTF-8 string path
		 * @param page Page to load
		 * @return error::Errorcode
		 */
		error::Errorcode pdfLoad(std::string_view path, std::size_t page = 1);
		/**
		 * @brief Loads PDF file from path given as UTF-16 string, loads given page, first page by default
		 * 
		 * @param path UTF-16 string path
		 * @param page Page to load
		 * @return error::Errorcode 
		 */
		error::Errorcode pdfLoad(const std::wstring & path, std::size_t page = 1);
		/**
		 * @brief Loads a PDF file from binary data, given as byte array, preserves the array,
		 * loads given page, first page by default
		 * 
		 * @param data Pointer to an array of bytes containing the PDF
		 * @param length Length of binary data
		 * @param page Page to load
		 * @return error::Errorcode 
		 */
		error::Errorcode pdfLoad(const u8 * data, std::size_t length, std::size_t page = 1);
		/**
		 * @brief Loads a PDF file from binary data, given as byte array, consumes the array,
		 * loads given page, first page by default
		 * 
		 * @param data Pointer to an array of bytes containing the PDF
		 * @param length Lengths of binary data
		 * @param page Page to load
		 * @return error::Errorcode 
		 */
		error::Errorcode pdfLoad(u8 * && data, std::size_t length, std::size_t page = 1) noexcept;
		/**
		 * @brief Unloads (closes) currently loaded PDF if any is open,
		 * also any pages that might be open
		 * 
		 */
		void pdfUnload() noexcept;

		/**
		 * @brief Loads given page from currently loaded PDF if any is open
		 * 
		 * @param page Page to load
		 * @return error::Errorcode 
		 */
		error::Errorcode pageLoad(std::size_t page) noexcept;
		/**
		 * @brief Unloads (closes) the currently loaded page if any is open
		 * 
		 */
		void pageUnload() noexcept;
		/**
		 * @brief Render the current page of the PDF to the specified position
		 * on the device context with the specified size
		 * 
		 * @param dc Device context
		 * @param pos Position of the page
		 * @param size Size of the page
		 * @return error::Errorcode 
		 */
		error::Errorcode pageRender(HDC dc, pdfv::xy<int> pos, pdfv::xy<int> size);

		/**
		 * @return std::size_t Page count of the currently open PDF, 0 if none is open
		 */
		[[nodiscard]] constexpr std::size_t pageGetCount() const noexcept
		{
			return this->m_numPages;
		}
		/**
		 * @return std::size_t Page number of currently open page, 0 if none is open
		 */
		[[nodiscard]] constexpr std::size_t pageGetNum() const noexcept
		{
			return this->m_fpagenum;
		}
		/**
		 * @return true PDF is loaded
		 * @return false PDF is not loaded
		 */
		[[nodiscard]] constexpr bool pdfExists() const noexcept
		{
			return this->m_fdoc != nullptr;
		}
	};
}
