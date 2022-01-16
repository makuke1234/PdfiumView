#include "lib.hpp"
#include "main_window.hpp"
#include <iostream>

pdfv::lib::lib() noexcept
{
	if (s_libInit == false)
		Init();
}
pdfv::lib::lib(lib&& other) noexcept
	: m_fdoc(other.m_fdoc), m_fpage(other.m_fpage),
	m_fpagenum(other.m_fpagenum), m_numPages(other.m_numPages),
	m_buf(std::move(other.m_buf))
{
	other.m_fdoc = nullptr;
	other.m_fpage = nullptr;
}
pdfv::lib& pdfv::lib::operator=(lib&& other) noexcept
{
	m_fdoc = other.m_fdoc;
	m_fpage = other.m_fpage;
	m_fpagenum = other.m_fpagenum;
	m_numPages = other.m_numPages;
	m_buf = std::move(other.m_buf);

	other.m_fdoc = nullptr;
	other.m_fpage = nullptr;

	return *this;
}
pdfv::lib::~lib() noexcept
{
	PdfUnload();
}

void pdfv::lib::Init() noexcept
{
	assert(s_libInit == false);
	
	FPDF_LIBRARY_CONFIG config{};
	config.version = 2;

	FPDF_InitLibraryWithConfig(&config);
	s_libInit = true;
}
void pdfv::lib::Destroy() noexcept
{
	assert(s_libInit == true);

	FPDF_DestroyLibrary();
	s_libInit = false;
}

[[nodiscard]] pdfv::error::errorcode pdfv::lib::GetLastError() noexcept
{
	assert(s_libInit == true);
	assert(s_errorHappened == true);

	s_errorHappened = false;
	return error::errorcode(FPDF_GetLastError() + error::pdf_success);
}
pdfv::error::errorcode pdfv::lib::PdfLoad(std::string_view path, std::size_t page) noexcept
{
	assert(s_libInit == true);
	PdfUnload();
	
	m_fdoc = FPDF_LoadDocument(path.data(), nullptr);
	if (m_fdoc == nullptr) {
		s_errorHappened = true;
		auto err = GetLastError();
		if (err == error::pdf_password) {
			auto ans = Convert(AskInfo(L"Enter password:", main_window::mwnd.GetTitle()));
			m_fdoc = FPDF_LoadDocument(path.data(), ans.c_str());
			if (m_fdoc == nullptr) {
				s_errorHappened = true;
				return GetLastError();
			}
		}
		else
			return err;
	}
	
	m_numPages = FPDF_GetPageCount(m_fdoc);
	return PageLoad(page);
}
pdfv::error::errorcode pdfv::lib::PdfLoad(std::wstring_view path, std::size_t page)
{
	assert(s_libInit == true);
	PdfUnload();

	auto file = ::CreateFileW(
		path.data(),
		GENERIC_READ,
		FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);
	auto size = ::GetFileSize(file, nullptr);
	m_buf.reset(new std::uint8_t[size]);
	DWORD read = 0;

	if (::ReadFile(file, m_buf.get(), size, &read, nullptr)) {
		::CloseHandle(file);

		m_fdoc = FPDF_LoadMemDocument(m_buf.get(), size, nullptr);
		if (m_fdoc == nullptr) {
			s_errorHappened = true;
			auto err = GetLastError();
			if (err == error::pdf_password) {
				auto ans = Convert(AskInfo(L"Enter password:", main_window::mwnd.GetTitle()));
				m_fdoc = FPDF_LoadMemDocument(m_buf.get(), size, ans.c_str());
				if (m_fdoc == nullptr) {
					s_errorHappened = true;
					return GetLastError();
				}
			}
			else
				return err;
		}

		m_numPages = FPDF_GetPageCount(m_fdoc);
		return PageLoad(page);
	}

	return error::pdf_file;
}
pdfv::error::errorcode pdfv::lib::PdfLoad(std::vector<std::uint8_t> const& data, std::size_t page) noexcept
{
	assert(s_libInit == true);
	PdfUnload();

	m_buf.reset(new std::uint8_t[data.size()]);
	std::copy(data.begin(), data.end(), m_buf.get());
	
	m_fdoc = FPDF_LoadMemDocument(m_buf.get(), data.size(), nullptr);
	if (m_fdoc == nullptr) {
		s_errorHappened = true;
		auto err = GetLastError();
		if (err == error::pdf_password) {
			auto ans = Convert(AskInfo(L"Enter password:", main_window::mwnd.GetTitle()));
			m_fdoc = FPDF_LoadMemDocument(m_buf.get(), data.size(), ans.c_str());
			if (m_fdoc == nullptr) {
				s_errorHappened = true;
				return GetLastError();
			}
		}
		else
			return err;
	}
	
	m_numPages = FPDF_GetPageCount(m_fdoc);
	return PageLoad(page);
}

void pdfv::lib::PdfUnload() noexcept
{
	assert(s_libInit == true);

	PageUnload();
	if (m_fdoc != nullptr) {
		FPDF_CloseDocument(m_fdoc);
		m_fdoc = nullptr;
		m_numPages = 0;
	}
}

pdfv::error::errorcode pdfv::lib::PageLoad(std::size_t page) noexcept
{
	assert(s_libInit == true);
	assert(page >= 1);
	assert(page <= m_numPages);
	assert(m_fdoc != nullptr);
	
	if (page != m_fpagenum) {
		PageUnload();
		m_fpage = FPDF_LoadPage(m_fdoc, page - 1);
		if (m_fpage == nullptr) {
			s_errorHappened = true;
			return GetLastError();
		}
		m_fpagenum = page;
	}

	return error::pdf_success;
}
void pdfv::lib::PageUnload() noexcept
{
	assert(s_libInit == true);

	if (m_fpage != nullptr) {
		FPDF_ClosePage(m_fpage);
		m_fpage = nullptr;
		m_fpagenum = 0;
	}
}

pdfv::error::errorcode pdfv::lib::RenderPage(HDC deviceContext, pdfv::xy<int> pos, pdfv::xy<int> size) const noexcept
{
	assert(s_libInit == true);

	if (m_fpage != nullptr) {
		double heightfactor = FPDF_GetPageHeight(m_fpage) / FPDF_GetPageWidth(m_fpage);
		
		pdfv::xy<int> newsize;
		int temp1 = size.y - 2 * pos.y;
		int temp2 = int(heightfactor * double(size.x - 2 * pos.x));
		newsize.y = std::min(temp1, temp2);
		newsize.x = int(double(newsize.y) / heightfactor);

		pos = size - newsize;
		pos /= 2;

		FPDF_RenderPage(deviceContext, m_fpage, pos.x, pos.y, newsize.x, newsize.y, 0, 0);

		return error::noerror;
	}
	else
		return error::pdf_page;
}
