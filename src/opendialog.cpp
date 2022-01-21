#include "opendialog.hpp"

#include <commdlg.h>

pdfv::OpenDialog::OpenDialog(std::size_t bufsize)
	: m_lastName(new wchar_t[bufsize]), m_bufSize(bufsize)
{
	DEBUGPRINT("pdfv::OpenDialog::OpenDialog(%zu)\n", bufsize);
	// Good practise
	this->m_lastName.get()[0] = L'\0';
	this->m_lastName.get()[bufsize - 1] = L'\0';
}
pdfv::OpenDialog::OpenDialog(std::wstring_view defaultName, pdfv::ssize_t bufsize)
{
	DEBUGPRINT("pdfv::OpenDialog::OpenDialog(%p, %zd)\n", static_cast<const void *>(defaultName.data()), bufsize);
	if (bufsize == -1)
	{
		this->m_bufSize = defaultName.length() + 1;
	}
	else
	{
		this->m_bufSize = bufsize;
	}
	
	this->m_lastName.reset(new wchar_t[this->m_bufSize]);
	this->m_lastName.get()[this->m_bufSize - 1] = L'\0';

	std::copy(defaultName.begin(), defaultName.end(), this->m_lastName.get());
	this->m_lastName.get()[defaultName.length()] = L'\0';
}
pdfv::OpenDialog::OpenDialog(wchar_t * && moveDefaultName, pdfv::ssize_t bufsize)
{
	DEBUGPRINT("pdfv::OpenDialog(%p, %zd)\n", static_cast<void *>(moveDefaultName), bufsize);
	this->m_lastName.reset(moveDefaultName);
	moveDefaultName = nullptr;

	if (bufsize == -1)
	{
		this->m_bufSize = std::wcslen(this->m_lastName.get()) + 1;
	}
	else
	{
		this->m_bufSize = bufsize;
	}
}
pdfv::OpenDialog::OpenDialog(const OpenDialog & other)
	: m_lastName(new wchar_t[other.m_bufSize]), m_bufSize(other.m_bufSize)
{
	DEBUGPRINT("pdfv::OpenDialog::OpenDialog(const OpenDialog & %p)\n", static_cast<const void *>(&other));
	std::copy(other.m_lastName.get(), other.m_lastName.get() + this->m_bufSize, this->m_lastName.get());
}
pdfv::OpenDialog::OpenDialog(OpenDialog && other) noexcept
	: m_lastName(std::move(other.m_lastName)), m_bufSize(other.m_bufSize)
{
	DEBUGPRINT("pdfv::OpenDialog::OpenDialog(OpenDialog && %p)\n", static_cast<void *>(&other));
	other.m_bufSize = 0;
}
pdfv::OpenDialog & pdfv::OpenDialog::operator=(const OpenDialog & other)
{
	DEBUGPRINT("pdfv::OpenDialog::operator=(const OpenDialog & %p)\n", static_cast<const void *>(&other));
	this->m_bufSize = other.m_bufSize;
	this->m_lastName.reset(new wchar_t[this->m_bufSize]);
	std::copy(other.m_lastName.get(), other.m_lastName.get() + this->m_bufSize, this->m_lastName.get());

	return *this;
}
pdfv::OpenDialog & pdfv::OpenDialog::operator=(OpenDialog && other) noexcept
{
	DEBUGPRINT("pdfv::OpenDialog::operator=(OpenDialog && %p)\n", static_cast<void *>(&other));
	this->m_lastName = std::move(other.m_lastName);
	this->m_bufSize  = other.m_bufSize;
	
	other.m_bufSize  = 0;

	return *this;
}

[[nodiscard]] bool pdfv::OpenDialog::open(HWND hwnd, std::wstring & output)
{
	DEBUGPRINT("pdfv::OpenDialog::open(%p, %p)\n", static_cast<void *>(hwnd), static_cast<const void *>(output.c_str()));
	output.assign(this->m_lastName.get());
	auto lastnameLen{ output.length() };
	output.resize(this->m_bufSize);
	
	OPENFILENAMEW ofn{};

	ofn.lStructSize  = sizeof ofn;
	ofn.hwndOwner    = hwnd;
	ofn.lpstrFilter  = s_cDefaultOpenFilter;
	ofn.nFilterIndex = s_cDefaultOpenFilterIndex;
	ofn.lpstrFile    = output.data();
	ofn.nMaxFile     = this->m_bufSize;
	ofn.lpstrTitle   = s_cDefaultOpenTitle;
	ofn.Flags        = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	ofn.lpstrDefExt  = L"pdf";

	bool ret{ ::GetOpenFileNameW(&ofn) ? true : false };
	if (ret) [[likely]]
	{
		output.resize(std::wcslen(output.c_str()));
		std::copy(output.begin(), output.end(), this->m_lastName.get());
		this->m_lastName.get()[output.length()] = L'\0';
	}
	else [[unlikely]]
	{
		output.assign(this->m_lastName.get(), lastnameLen);
	}

	return ret;
}

bool pdfv::OpenDialog::updateName(std::wstring_view newfilename) noexcept
{
	DEBUGPRINT("pdfv::OpenDialog::updateName(%p)\n", static_cast<const void *>(newfilename.data()));
	if (this->m_bufSize <= (newfilename.length() + 1)) [[likely]]
	{
		std::copy(newfilename.begin(), newfilename.end(), this->m_lastName.get());
		this->m_lastName.get()[newfilename.length()] = L'\0';
		return true;
	}
	else [[unlikely]]
	{
		return false;
	}
}
