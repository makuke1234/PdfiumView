#pragma once

#include "concepts.hpp"

namespace pdfv::w
{
	template<concepts::pointer T, typename Deleter>
	struct Safeptr
	{
		T obj{ nullptr };
		Deleter deleter;

		Safeptr() = default;
		constexpr Safeptr(const T & value) noexcept
			: obj(value)
		{
		}
		constexpr Safeptr(const Safeptr & other) noexcept
			: obj(other.obj)
		{
		}
		constexpr Safeptr(Safeptr && other) noexcept
			: obj(other.obj)
		{
			other.obj = nullptr;
		}
		Safeptr & operator=(const Safeptr & other) noexcept
		{
			this->~Safeptr();
			this->obj = other.obj;
			return *this;
		}
		Safeptr & operator=(Safeptr && other) noexcept
		{
			this->~Safeptr();
			this->obj = other.obj;
			other.obj = nullptr;
			return *this;
		}
		~Safeptr() noexcept
		{
			if (this->obj != nullptr)
			{
				auto temp{ this->obj };
				this->obj = nullptr;
				this->deleter(temp);
			}
		}

		[[nodiscard]] constexpr operator T & () noexcept
		{
			return this->obj;
		}
		[[nodiscard]] constexpr operator const T & () const noexcept
		{
			return this->obj;
		}
		[[nodiscard]] constexpr T & get() noexcept
		{
			return this->obj;
		}
		[[nodiscard]] constexpr const T & get() const noexcept
		{
			return this->obj;
		}
		[[nodiscard]] constexpr T * operator->() noexcept
		{
			return &this->obj;
		}
	};
}
