#include "otherwindow.hpp"
#include "mainwindow.hpp"

pdfv::OtherWindow::OtherWindow(std::wstring_view fileName) noexcept
{
	this->mtx = ::CreateMutexW(nullptr, FALSE, APP_CLASSNAME);
	if (::GetLastError() == ERROR_ALREADY_EXISTS)
	{
		::ReleaseMutex(this->mtx);
		this->mtx = nullptr;

		if (auto otherwindow = ::FindWindow(APP_CLASSNAME, nullptr); otherwindow != nullptr)
		{
			if (fileName.length() != 0)
			{
				COPYDATASTRUCT cds{};
				cds.dwData = 1;
				cds.cbData = sizeof(wchar_t) * (fileName.length() + 1);
				cds.lpData = const_cast<wchar_t *>(fileName.data());

				::SendMessageW(
					otherwindow,
					WM_COPYDATA,
					0,
					reinterpret_cast<LPARAM>(&cds)
				);
			}

			::SendMessageW(
				otherwindow,
				pdfv::MainWindow::WM_BRINGTOFRONT,
				0,
				0
			);
		}
	}
}
pdfv::OtherWindow::~OtherWindow() noexcept
{
	if (this->mtx != nullptr)
	{
		::ReleaseMutex(this->mtx);
		this->mtx = nullptr;
	}
}
