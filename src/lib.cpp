#include "lib.hpp"
#include "mainwindow.hpp"
#include <iostream>

static struct PdfiumFree
{
	~PdfiumFree() noexcept
	{
		pdfv::Pdfium::free();
	}

} s_pdfiumFree;


pdfv::Pdfium::Pdfium() noexcept
{
	DEBUGPRINT("pdfv::Pdfium::Pdfium()\n");

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
	DEBUGPRINT("pdfv::Pdfium::Pdfium(%p)\n", static_cast<void *>(&other));
	other.m_fdoc  = nullptr;
	other.m_fpage = nullptr;
}
pdfv::Pdfium & pdfv::Pdfium::operator=(Pdfium && other) noexcept
{
	DEBUGPRINT("pdfv::Pdfium::operator=(%p)\n", static_cast<void *>(&other));

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
	DEBUGPRINT("pdfv::Pdfium::~Pdfium()\n");

	this->pdfUnload();
}

void pdfv::Pdfium::init() noexcept
{
	DEBUGPRINT("pdfv::Pdfium::init()\n");

	assert(s_libInit == false);
	
	FPDF_LIBRARY_CONFIG config{};
	config.version = 2;

	FPDF_InitLibraryWithConfig(&config);
	s_libInit = true;
}
void pdfv::Pdfium::free() noexcept
{
	DEBUGPRINT("pdfv::Pdfium::free()\n");
	if (s_libInit == false)
	{
		return;
	}

	// Free library as usual
	FPDF_DestroyLibrary();
	s_libInit = false;
}

[[nodiscard]] pdfv::error::Errorcode pdfv::Pdfium::getLastError() noexcept
{
	DEBUGPRINT("pdfv::Pdfium::getLastError()\n");
	assert(s_libInit == true);
	assert(s_errorHappened == true);

	s_errorHappened = false;
	return error::Errorcode(FPDF_GetLastError() + error::pdf_success);
}
pdfv::error::Errorcode pdfv::Pdfium::pdfLoad(std::string_view path, std::size_t page)
{
	DEBUGPRINT("pdfv::Pdfium::pdfLoad(%p, %zu)\n", static_cast<const void *>(path.data()), page);
	assert(s_libInit == true);
	this->pdfUnload();

	return this->pdfLoad(utf::conv(path), page);
}
pdfv::error::Errorcode pdfv::Pdfium::pdfLoad(const std::wstring & path, std::size_t page)
{
	DEBUGPRINT("pdfv::Pdfium::pdfLoad(%p, %zu)\n", static_cast<const void *>(path.c_str()), page);
	assert(s_libInit == true);
	this->pdfUnload();

	auto file{ ::CreateFileW(
		path.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	) };
	if (file == INVALID_HANDLE_VALUE) [[unlikely]]
	{
		return error::pdf_file;
	}

	auto size{ ::GetFileSize(file, nullptr) };
	auto buf{ new std::uint8_t[size] };
	DWORD read{ 0 };

	if (::ReadFile(file, buf, size, &read, nullptr))
	{
		::CloseHandle(file);
		return this->pdfLoad(std::move(buf), std::size_t(read), page);
	}

	::CloseHandle(file);

	return error::pdf_file;
}
pdfv::error::Errorcode pdfv::Pdfium::pdfLoad(const u8 * data, std::size_t length, std::size_t page)
{
	DEBUGPRINT("pdfv::Pdfium::pdfLoad(%p, %zu, %zu)\n", static_cast<const void *>(data), length, page);
	assert(s_libInit == true);
	this->pdfUnload();

	auto buf{ new u8[length] };
	std::copy(data, data + length, buf);
	
	return this->pdfLoad(std::move(buf), length, page);
}
pdfv::error::Errorcode pdfv::Pdfium::pdfLoad(u8 * && data, std::size_t length, std::size_t page) noexcept
{
	DEBUGPRINT("pdfv::Pdfium::pdfLoad(&& %p, %zu, %zu)\n", static_cast<void *>(data), length, page);
	assert(s_libInit == true);
	this->pdfUnload();

	this->m_buf.reset(data);
	
	this->m_fdoc = FPDF_LoadMemDocument(this->m_buf.get(), length, nullptr);
	if (this->m_fdoc == nullptr) [[unlikely]]
	{
		s_errorHappened = true;
		if (auto err{ this->getLastError() }; err == error::pdf_password)
		{
			auto ans{ utf::conv(askInfo(L"Enter password:", MainWindow::mwnd.getTitle())) };
			this->m_fdoc = FPDF_LoadMemDocument(this->m_buf.get(), length, ans.c_str());
			if (this->m_fdoc == nullptr) [[unlikely]]
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

void pdfv::Pdfium::pdfUnload() noexcept
{
	DEBUGPRINT("pdfv::Pdfium::pdfUnload()\n");
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
	DEBUGPRINT("pdfv::Pdfium::pageLoad(%zu)\n", page);
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
	DEBUGPRINT("pdfv::Pdfium::pageUnload()\n");
	assert(s_libInit == true);

	if (this->m_fpage != nullptr)
	{
		FPDF_ClosePage(this->m_fpage);
		this->m_fpage    = nullptr;
		this->m_fpagenum = 0;
	}
}

pdfv::error::Errorcode pdfv::Pdfium::pageRender(HDC dc, pdfv::xy<int> pos, pdfv::xy<int> size)
{
	DEBUGPRINT("pdfv::Pdfium::pageRender(%p, %p, %p)\n", static_cast<void *>(dc), static_cast<void *>(&pos), static_cast<void *>(&size));
	assert(s_libInit == true);

	if (this->m_fpage != nullptr)
	{
		auto heightfactor{ FPDF_GetPageHeight(this->m_fpage) / FPDF_GetPageWidth(this->m_fpage) };
		
		pdfv::xy<int> newsize;
		auto temp1{ size.y - 2 * pos.y };
		auto temp2{ int(heightfactor * double(size.x - 2 * pos.x)) };
		newsize.y = std::min(temp1, temp2);
		newsize.x = int(double(newsize.y) / heightfactor);

		pos = (size - newsize) / 2;

		void * args[]{ this, dc, &newsize };
		this->m_optRenderer.putPage(
			this->m_fpagenum,
			pos,
			newsize,
			[](void * args) -> hdc::Renderer::RenderT
			{
				DEBUGPRINT("render!\n");
				auto argv{ reinterpret_cast<void **>(args) };

				auto self{ static_cast<Pdfium *>(argv[0]) };
				auto dc  { static_cast<HDC>(argv[1]) };
				auto size{ static_cast<xy<int> *>(argv[2]) };

				auto memdc { ::CreateCompatibleDC(dc) };
				auto render{ ::CreateCompatibleBitmap(dc, size->x, size->y) };

				auto hbmold{ ::SelectObject(memdc, render) };

				RECT r{ .left = 0, .top = 0, .right = size->x, .bottom = size->y };
				::FillRect(memdc, &r, static_cast<HBRUSH>(::GetStockObject(WHITE_BRUSH)));

				FPDF_RenderPage(memdc, self->m_fpage, 0, 0, size->x, size->y, 0, 0);

				::SelectObject(memdc, hbmold);
				::DeleteDC(memdc);

				return render;
			},
			args
		);
		
		const auto & render{ this->m_optRenderer.getPage(this->m_fpagenum) };

		auto memdc{ ::CreateCompatibleDC(dc) };
		DEBUGPRINT("HBITMAP = %p\n", static_cast<void *>(render.get()));
		auto hbmold{ ::SelectObject(memdc, render.get()) };

		::BitBlt(dc, pos.x, pos.y, newsize.x, newsize.y, memdc, 0, 0, SRCCOPY);

		::SelectObject(memdc, hbmold);
		::DeleteDC(memdc);

		return error::noerror;
	}
	else
	{
		return error::pdf_page;
	}
}
