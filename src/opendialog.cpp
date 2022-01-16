#include "opendialog.hpp"

#include <commdlg.h>

pdfv::OpenDialog::OpenDialog(std::size_t bufsize)
	: m_lastName(new wchar_t[bufsize]), m_currentName(new wchar_t[bufsize]),
	m_bufSize(bufsize)
{
	// Good practise
	this->m_lastName.get()[0] = L'\0';
	this->m_lastName.get()[bufsize - 1] = L'\0';
}
pdfv::OpenDialog::OpenDialog(std::wstring_view defaultName, pdfv::ssize_t bufsize)
{
	if (bufsize == -1)
	{
		this->m_bufSize = defaultName.length() + 1;
	}
	else
	{
		this->m_bufSize = bufsize;
	}
	
	this->m_lastName.reset(new wchar_t[this->m_bufSize]);
	this->m_lastName.get()[this->m_bufSize - 1] = 0;
	this->m_currentName.reset(new wchar_t[this->m_bufSize]);

	std::copy(defaultName.begin(), defaultName.end() + 1, this->m_lastName.get());

}
pdfv::OpenDialog::OpenDialog(wchar_t * && moveDefaultName, pdfv::ssize_t bufsize)
{
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
	
	this->m_currentName.reset(new wchar_t[this->m_bufSize]);
}
pdfv::OpenDialog::OpenDialog(const OpenDialog & other)
	: m_lastName(new wchar_t[other.m_bufSize]),
	m_currentName(new wchar_t[other.m_bufSize]), m_bufSize(other.m_bufSize)
{
	std::copy(other.m_lastName.get(), other.m_lastName.get() + this->m_bufSize, this->m_lastName.get());
}
pdfv::OpenDialog::OpenDialog(OpenDialog && other) noexcept
	: m_lastName(std::move(other.m_lastName)),
	m_currentName(std::move(other.m_currentName)), m_bufSize(other.m_bufSize)
{
	other.m_bufSize = 0;
}
pdfv::OpenDialog & pdfv::OpenDialog::operator=(const OpenDialog & other)
{
	this->m_bufSize = other.m_bufSize;
	this->m_lastName.reset(new wchar_t[this->m_bufSize]);
	std::copy(other.m_lastName.get(), other.m_lastName.get() + this->m_bufSize, this->m_lastName.get());
	this->m_currentName.reset(new wchar_t[m_bufSize]);

	return *this;
}
pdfv::OpenDialog & pdfv::OpenDialog::operator=(OpenDialog && other) noexcept
{
	this->m_lastName    = std::move(other.m_lastName);
	this->m_currentName = std::move(other.m_currentName);
	this->m_bufSize     = other.m_bufSize;
	
	other.m_bufSize = 0;

	return *this;
}

[[nodiscard]] bool pdfv::OpenDialog::open(HWND hwnd, std::wstring & output)
{
	std::copy(this->m_lastName.get(), this->m_lastName.get() + this->m_bufSize, this->m_currentName.get());
	
	OPENFILENAMEW ofn{};

	ofn.lStructSize  = sizeof ofn;
	ofn.hwndOwner    = hwnd;
	ofn.lpstrFilter  = s_cDefaultOpenFilter;
	ofn.nFilterIndex = s_cDefaultOpenFilterIndex;
	ofn.lpstrFile    = this->m_currentName.get();
	ofn.nMaxFile     = this->m_bufSize;
	ofn.lpstrTitle   = s_cDefaultOpenTitle;
	ofn.Flags        = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	ofn.lpstrDefExt  = L"pdf";

	bool ret = ::GetOpenFileNameW(&ofn) != 0;
	if (ret)
	{
		output.assign(this->m_currentName.get());
		std::copy(this->m_currentName.get(), this->m_currentName.get() + output.length() + 1, this->m_lastName.get());
	}
	else
	{
		output.assign(this->m_lastName.get());
	}

	return ret;
}

bool pdfv::OpenDialog::updateName(std::wstring_view newfilename) noexcept
{
	if (this->m_bufSize <= (newfilename.length() + 1))
	{
		std::copy(newfilename.begin(), newfilename.end() + 1, this->m_lastName.get());
		return true;
	}
	else
	{
		return false;
	}
}
