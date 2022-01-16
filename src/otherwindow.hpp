#pragma once

#include "common.hpp"

namespace pdfv
{
	class OtherWindow
	{
	private:
		HANDLE mtx{ nullptr };

	public:
		OtherWindow(std::wstring_view fileName) noexcept;
		~OtherWindow() noexcept;

		[[nodiscard]] constexpr bool exists() const noexcept
		{
			return this->mtx == nullptr;
		}
	};

}
