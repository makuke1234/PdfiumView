#pragma once

#include "common.hpp"

namespace pdfv
{
	class OtherWindow
	{
	private:
		HANDLE mtx{ nullptr };

	public:
		/**
		 * @brief Construct a new OtherWindow object
		 * 
		 * @param fileName Name of the file for the other window to open
		 */
		OtherWindow(std::wstring_view fileName) noexcept;
		~OtherWindow() noexcept;

		/**
		 * @return true Application is already open, named mutex already exists
		 */
		[[nodiscard]] constexpr bool exists() const noexcept
		{
			return this->mtx == nullptr;
		}
	};

}
