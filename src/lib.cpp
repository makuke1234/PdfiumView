#include "lib.hpp"
#include "mainwindow.hpp"
#include <iostream>

pdfv::Pdfium::Pdfium() noexcept
{
	if (s_libInit == false)
	{
		this->init();
	}
}
pdfv::Pdfium::Pdfium(Pdfium && other) noexcept
	: m_fdoc(other.m_fdoc), m_fpage(other.m_fpage),
	m_fpagenum(other.m_fpagenum), m_numPages(other.m_numPages),
	m_buf(std::move(other.m_buf))
{
	other.m_fdoc  = nullptr;
	other.m_fpage = nullptr;
}
pdfv::Pdfium & pdfv::Pdfium::operator=(Pdfium && other) noexcept
{
	this->m_fdoc     = other.m_fdoc;
	this->m_fpage    = other.m_fpage;
	this->m_fpagenum = other.m_fpagenum;
	this->m_numPages = other.m_numPages;
	this->m_buf      = std::move(other.m_buf);

	other.m_fdoc  = nullptr;
	other.m_fpage = nullptr;

	return *this;
}
pdfv::Pdfium::~Pdfium() noexcept
{
	this->pdfUnload();
}

void pdfv::Pdfium::init() noexcept
{
	assert(s_libInit == false);
	
	FPDF_LIBRARY_CONFIG config{};
	config.version = 2;

	FPDF_InitLibraryWithConfig(&config);
	s_libInit = true;
}
void pdfv::Pdfium::free() noexcept
{
	assert(s_libInit == true);

	FPDF_DestroyLibrary();
	s_libInit = false;
}

[[nodiscard]] pdfv::error::Errorcode pdfv::Pdfium::getLastError() noexcept
{
	assert(s_libInit == true);
	assert(s_errorHappened == true);

	s_errorHappened = false;
	return error::Errorcode(FPDF_GetLastError() + error::pdf_success);
}
pdfv::error::Errorcode pdfv::Pdfium::pdfLoad(std::string_view path, std::size_t page) noexcept
{
	assert(s_libInit == true);
	this->pdfUnload();
	
	this->m_fdoc = FPDF_LoadDocument(path.data(), nullptr);
	if (this->m_fdoc == nullptr)
	{
		s_errorHappened = true;
		auto err = this->getLastError();
		if (err == error::pdf_password)
		{
			auto ans = utf::conv(askInfo(L"Enter password:", MainWindow::mwnd.getTitle()));
			this->m_fdoc = FPDF_LoadDocument(path.data(), ans.c_str());
			if (this->m_fdoc == nullptr)
			{
				s_errorHappened = true;
				return this->getLastError();
			}
		}
		else
		{
			return err;
		}
	}
	
	this->m_numPages = FPDF_GetPageCount(this->m_fdoc);
	return this->pageLoad(page);
}
pdfv::error::Errorcode pdfv::Pdfium::pdfLoad(std::wstring_view path, std::size_t page)
{
	assert(s_libInit == true);

	auto nPath = utf::conv(path);
	return this->pdfLoad(nPath, page);
}
pdfv::error::Errorcode pdfv::Pdfium::pdfLoad(const std::vector<std::uint8_t> & data, std::size_t page) noexcept
{
	assert(s_libInit == true);
	this->pdfUnload();

	this->m_buf.reset(new std::uint8_t[data.size()]);
	std::copy(data.begin(), data.end(), this->m_buf.get());
	
	this->m_fdoc = FPDF_LoadMemDocument(this->m_buf.get(), data.size(), nullptr);
	if (this->m_fdoc == nullptr)
	{
		s_errorHappened = true;
		auto err = getLastError();
		if (err == error::pdf_password) 
		{
			auto ans = utf::conv(askInfo(L"Enter password:", MainWindow::mwnd.getTitle()));
			this->m_fdoc = FPDF_LoadMemDocument(this->m_buf.get(), data.size(), ans.c_str());
			if (this->m_fdoc == nullptr)
			{
				s_errorHappened = true;
				return getLastError();
			}
		}
		else
		{
			return err;
		}
	}
	
	this->m_numPages = FPDF_GetPageCount(this->m_fdoc);
	return this->pageLoad(page);
}

void pdfv::Pdfium::pdfUnload() noexcept
{
	assert(s_libInit == true);

	this->pageUnload();
	if (this->m_fdoc != nullptr)
	{
		FPDF_CloseDocument(this->m_fdoc);
		this->m_fdoc     = nullptr;
		this->m_numPages = 0;
	}
}

pdfv::error::Errorcode pdfv::Pdfium::pageLoad(std::size_t page) noexcept
{
	assert(s_libInit == true);
	assert(page >= 1);
	assert(page <= this->m_numPages);
	assert(this->m_fdoc != nullptr);
	
	if (page != this->m_fpagenum)
	{
		this->pageUnload();
		this->m_fpage = FPDF_LoadPage(this->m_fdoc, page - 1);
		if (this->m_fpage == nullptr)
		{
			s_errorHappened = true;
			return this->getLastError();
		}
		this->m_fpagenum = page;
	}

	return error::pdf_success;
}
void pdfv::Pdfium::pageUnload() noexcept
{
	assert(s_libInit == true);

	if (this->m_fpage != nullptr)
	{
		FPDF_ClosePage(this->m_fpage);
		this->m_fpage    = nullptr;
		this->m_fpagenum = 0;
	}
}

pdfv::error::Errorcode pdfv::Pdfium::pageRender(HDC dc, pdfv::xy<int> pos, pdfv::xy<int> size) const noexcept
{
	assert(s_libInit == true);

	if (this->m_fpage != nullptr)
	{
		double heightfactor = FPDF_GetPageHeight(this->m_fpage) / FPDF_GetPageWidth(this->m_fpage);
		
		pdfv::xy<int> newsize;
		int temp1 = size.y - 2 * pos.y;
		int temp2 = int(heightfactor * double(size.x - 2 * pos.x));
		newsize.y = std::min(temp1, temp2);
		newsize.x = int(double(newsize.y) / heightfactor);

		pos  = size - newsize;
		pos /= 2;

		FPDF_RenderPage(dc, this->m_fpage, pos.x, pos.y, newsize.x, newsize.y, 0, 0);

		return error::noerror;
	}
	else
	{
		return error::pdf_page;
	}
}
