#include "opendialog.hpp"

#include <commdlg.h>

pdfv::opendialog::opendialog(std::size_t bufsize)
	: m_lastName(new wchar_t[bufsize]), m_currentName(new wchar_t[bufsize]),
	m_bufSize(bufsize)
{
	// Good practise
	m_lastName.get()[0] = 0;
	m_lastName.get()[bufsize - 1] = 0;
}
pdfv::opendialog::opendialog(std::wstring_view defaultName, pdfv::ssize_t bufsize)
{
	if (bufsize == -1) {
		m_bufSize = defaultName.length() + 1;
	}
	else
		m_bufSize = bufsize;
	
	m_lastName.reset(new wchar_t[m_bufSize]);
	m_lastName.get()[m_bufSize - 1] = 0;
	m_currentName.reset(new wchar_t[m_bufSize]);

	std::copy(defaultName.begin(), defaultName.end() + 1, m_lastName.get());

}
pdfv::opendialog::opendialog(wchar_t*&& moveDefaultName, pdfv::ssize_t bufsize)
{
	m_lastName.reset(moveDefaultName);
	moveDefaultName = nullptr;

	if (bufsize == -1)
		m_bufSize = std::wcslen(m_lastName.get()) + 1;
	else
		m_bufSize = bufsize;
	
	m_currentName.reset(new wchar_t[m_bufSize]);
}
pdfv::opendialog::opendialog(opendialog const& other)
	: m_lastName(new wchar_t[other.m_bufSize]),
	m_currentName(new wchar_t[other.m_bufSize]), m_bufSize(other.m_bufSize)
{
	std::copy(other.m_lastName.get(), other.m_lastName.get() + m_bufSize, m_lastName.get());
}
pdfv::opendialog::opendialog(opendialog&& other) noexcept
	: m_lastName(std::move(other.m_lastName)),
	m_currentName(std::move(other.m_currentName)), m_bufSize(other.m_bufSize)
{
	other.m_bufSize = 0;
}
pdfv::opendialog& pdfv::opendialog::operator=(opendialog const& other)
{
	m_bufSize = other.m_bufSize;
	m_lastName.reset(new wchar_t[m_bufSize]);
	std::copy(other.m_lastName.get(), other.m_lastName.get() + m_bufSize, m_lastName.get());
	m_currentName.reset(new wchar_t[m_bufSize]);

	return *this;
}
pdfv::opendialog& pdfv::opendialog::operator=(opendialog&& other) noexcept
{
	m_lastName = std::move(other.m_lastName);
	m_currentName = std::move(other.m_currentName);
	m_bufSize = other.m_bufSize;
	
	other.m_bufSize = 0;

	return *this;
}

[[nodiscard]] bool pdfv::opendialog::Open(HWND hwnd, std::wstring& output)
{
	std::copy(m_lastName.get(), m_lastName.get() + m_bufSize, m_currentName.get());
	
	OPENFILENAMEW ofn{};

	ofn.lStructSize = sizeof ofn;
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = s_cDefaultOpenFilter;
	ofn.nFilterIndex = s_cDefaultOpenFilterIndex;
	ofn.lpstrFile = m_currentName.get();
	ofn.nMaxFile = m_bufSize;
	ofn.lpstrTitle = s_cDefaultOpenTitle;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	ofn.lpstrDefExt = L"pdf";

	bool ret = ::GetOpenFileNameW(&ofn) != 0;
	if (ret) {
		output.assign(m_currentName.get());
		std::copy(m_currentName.get(), m_currentName.get() + output.length() + 1, m_lastName.get());
	}
	else
		output.assign(m_lastName.get());

	return ret;
}

bool pdfv::opendialog::UpdateName(std::wstring_view newfilename) noexcept
{
	if (m_bufSize <= (newfilename.length() + 1)) {
		std::copy(newfilename.begin(), newfilename.end() + 1, m_lastName.get());
		return true;
	}
	else
		return false;
}
