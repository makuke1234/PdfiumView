#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <commctrl.h>
#include <fpdfview.h>
#include <cassert>
#include <utility>
#include <cstdint>
#include <string>
#include <string_view>
#include <memory>
#include <functional>

#include "concepts.hpp"
#include "errors.hpp"
#include "version.hpp"
#include "resource.hpp"

#define APP_DEFSIZE_X 350
#define APP_DEFSIZE_Y 200

#if INTPTR_MAX == INT64_MAX
#define PTRPRINT "0x%016X"
#elif INTPTR_MAX == INT32_MAX
#define PTRPRINT "0x%08X"
#elif INTPTR_MAX == INT16_MAX
#define PTRPRINT "0x%04X"
#endif


#ifdef _DEBUG
	#include <cstdio>
	#define DEBUGPRINT(...) std::printf(__VA_ARGS__)

	#define DEBUGFUNCTION(f) f()
#else
	#define DEBUGPRINT(...)
	
	#define DEBUGFUNCTION(f)
#endif


namespace pdfv
{
	// Some type aliases
	using ssize_t = std::intptr_t;
	using uchar   = unsigned char;
	using ushort  = unsigned short;
	using uint    = unsigned int;
	using l       = long;
	using ul      = unsigned long;
	using ll      = long long;
	using ull     = unsigned long long;
	using f       = float;
	using d       = double;
	using ld      = long double;

	using i8  = std::int8_t;
	using u8  = std::uint8_t;
	using i16 = std::int16_t;
	using u16 = std::uint16_t;
	using i32 = std::int32_t;
	using u32 = std::uint32_t;
	using i64 = std::int64_t;
	using u64 = std::uint64_t;
	
	namespace w
	{
		[[nodiscard]] RECT getCliR(HWND hwnd, RECT def = {}) noexcept;
		[[nodiscard]] RECT getWinR(HWND hwnd, RECT def = {}) noexcept;

		constexpr auto getClientRect = getCliR;
		constexpr auto getWindowRect = getWinR;

		[[nodiscard]] std::wstring getWinText(HWND hwnd, const std::wstring & def = {});

		constexpr auto getWindowText = getWinText;

		void setFont(HWND hwnd, HFONT hfont, bool redraw = false) noexcept;
	}

	extern std::function<void(wchar_t **)> getArgsFree;
	[[nodiscard]] std::unique_ptr<wchar_t *, decltype(pdfv::getArgsFree)> getArgs(LPWSTR cmdLine, int & argc) noexcept;
	
	[[nodiscard]] bool initCC() noexcept;

	//
	//	Data structure that represents x and y coordinate pair of a point
	//	Provides some modern operator overloading
	//
	template<typename T = int>
	struct xy
	{
		T x{}, y{};

		xy() noexcept = default;
		constexpr xy(T x_, T y_) noexcept
			: x(x_), y(y_)
		{
		}
		constexpr xy(const RECT & r) noexcept requires std::is_constructible_v<T, decltype(RECT().left)>
			: x(T(r.right - r.left)), y(T(r.bottom - r.top))
		{
		}
		template<typename U>
		constexpr xy(const xy<U> & other) noexcept requires std::is_convertible_v<U, T> && concepts::floating_or_integral<T, U>
			: x(T(other.x)), y(T(other.y))
		{
		}
		template<typename U>
		explicit constexpr xy(const xy<U> & other) noexcept requires std::is_convertible_v<U, T> && concepts::different_trivial<T, U>
			: x(static_cast<T>(other.x)), y(static_cast<T>(other.y))
		{
		}
		constexpr xy(const xy & other) noexcept
			: x(other.x), y(other.y)
		{
		}
		constexpr xy(xy && other) noexcept
			: x(std::move(other.x)), y(std::move(other.y))
		{
		}
		constexpr xy & operator=(const xy & other) noexcept
		{
			x = other.x;
			y = other.y;
			return *this;
		}
		constexpr xy & operator=(xy && other) noexcept
		{
			x = std::move(other.x);
			y = std::move(other.y);
			return *this;
		}
		~xy() noexcept = default;
		//
		//	Swaps current object with another class object
		//
		constexpr void swap(xy & other) noexcept
		{
			xy temp{ std::move(other) };
			other = std::move(*this);
			*this = std::move(temp);
		}

		/**
		 * @brief Tells if values of value pairs are close enough (both values)
		 * 
		 * @param rhs Second value pair
		 * @param epsilon Precision of closeness
		 */
		[[nodiscard]] constexpr bool isclose(const xy & rhs, const T epsilon) noexcept
		{
			return (std::abs(x - rhs.x) <= epsilon) && (std::abs(y - rhs.y) <= epsilon);
		}
		//
		//	Tells if value pairs are equal to each other
		//
		[[nodiscard]] constexpr bool operator==(const xy & rhs) noexcept
		{
			return (x == rhs.x) && (y == rhs.y);
		}
		//
		//	Tells if value pairs are not equal to each other
		//
		[[nodiscard]] constexpr bool operator!=(const xy & rhs) noexcept
		{
			return (x != rhs.x) || (y != rhs.y);
		}
		//
		//	Tells if both members of current value pair are smaller than
		//	the members of the other value pair 
		//
		[[nodiscard]] constexpr bool operator< (const xy & rhs) noexcept
		{
			return (x < rhs.x) && (y < rhs.y);
		}
		//
		//	Tells if both members of current value pair are smaller than or
		//	equal to the members of the other value pair 
		//
		[[nodiscard]] constexpr bool operator<=(const xy & rhs) noexcept
		{
			return (x <= rhs.x) && (y <= rhs.y);
		}
		//
		//	Tells if both members of current value pair are larger than
		//	the members of the other value pair 
		//
		[[nodiscard]] constexpr bool operator> (const xy & rhs) noexcept
		{
			return (x > rhs.x) && (y > rhs.y);
		}
		//
		//	Tells if both members of current value pair are larger than or
		//	equal to the members of the other value pair 
		//
		[[nodiscard]] constexpr bool operator>=(const xy & rhs) noexcept
		{
			return (x >= rhs.x) && (y >= rhs.y);
		}

		//
		//	Adds up both members of value pairs and makes the answer a new
		//	value pair
		//
		[[nodiscard]] constexpr xy operator+ (const xy & rhs) noexcept
		{
			return { x + rhs.x, y + rhs.y };
		}
		//
		//	Subtracts both members of the other value pair from
		//	current value pair and makes the answer a new value pair
		//
		[[nodiscard]] constexpr xy operator- (const xy & rhs) noexcept
		{
			return { x - rhs.x, y - rhs.y };
		}
		//
		//	Multiplies both members of the other value pair with current
		//	value pair members and makes the answer a new value pair
		//
		[[nodiscard]] constexpr xy operator* (const xy & rhs) noexcept
		{
			return { x * rhs.x, y * rhs.y };
		}
		//
		//	Divides both members of the current value pair with the members
		//	of the other value pair and makes the answer a new value pair
		//
		[[nodiscard]] constexpr xy operator/ (const xy & rhs) noexcept
		{
			return { x / rhs.x, y / rhs.y };
		}
		
		//
		//	Adds both members of the other value pair to the current value pair
		//
		constexpr xy & operator+=(const xy & rhs) noexcept
		{
			x += rhs.x;
			y += rhs.y;
			return *this;
		}
		//
		//	Subtracts both members of the other value pair from the
		//	current value pair
		//
		constexpr xy & operator-=(const xy & rhs) noexcept
		{
			x -= rhs.x;
			y -= rhs.y;
			return *this;
		}
		//
		//	Multiplies the current value pair members
		//	with the other value pair members
		//
		constexpr xy & operator*=(const xy & rhs) noexcept
		{
			x *= rhs.x;
			y *= rhs.y;
			return *this;
		}
		//
		//	Divides the other value pair members
		//	from the current value pair
		//
		constexpr xy & operator/=(const xy & rhs) noexcept
		{
			x /= rhs.x;
			y /= rhs.y;
			return *this;
		}
		//
		//	Bitshifts the current value pair members to left
		//	with the members of the other value pair
		//
		constexpr xy & operator<<=(const xy & rhs) noexcept requires std::is_integral_v<T>
		{
			x <<= rhs.x;
			y <<= rhs.y;
			return *this;
		}
		//
		//	Bitshift the current value pair members to right
		//	with the members of the other value pair
		//
		constexpr xy & operator>>=(const xy & rhs) noexcept requires std::is_integral_v<T>
		{
			x >>= rhs.x;
			y >>= rhs.y;
			return *this;
		}


		//
		//	Adds a value to both members of the current value pair
		//
		constexpr xy & operator+=(const T & rhs) noexcept
		{
			x += rhs;
			y += rhs;
			return *this;
		}
		//
		//	Subtracts a value from both members of the current value pair
		//
		constexpr xy & operator-=(const T & rhs) noexcept
		{
			x -= rhs;
			y -= rhs;
			return *this;
		}
		//
		//	Multiplies a value with both members of the current value pair
		//
		constexpr xy & operator*=(const T & rhs) noexcept
		{
			x *= rhs;
			y *= rhs;
			return *this;
		}
		//
		//	Divides a value from both members of the current value pair
		//
		constexpr xy & operator/=(const T & rhs) noexcept
		{
			x /= rhs;
			y /= rhs;
			return *this;
		}
		//
		//	Bitshifts the current value pair members to left
		//	with the given value
		//
		constexpr xy & operator<<=(const T & rhs) noexcept requires std::is_integral_v<T>
		{
			x <<= rhs;
			y <<= rhs;
			return *this;
		}
		//
		//	Bitshift the current value pair members to right
		//	with the given value
		//
		constexpr xy & operator>>=(const T & rhs) noexcept requires std::is_integral_v<T>
		{
			x >>= rhs;
			y >>= rhs;
			return *this;
		}
	};

	[[nodiscard]] xy<decltype(RECT().left)> constexpr make_xy(const RECT & r) noexcept
	{
		return r;
	}
	
	extern xy<f> dpi;
	
	//
	//	Calculates correct screen coordinate according to a given coordinate
	//	and a given scaling factor
	//
	[[nodiscard]] constexpr int dip(const int size, const f dip) noexcept
	{
		return int(f(size) * dip);
	}
	//
	//	Calcluates correct screen coordinate pair according
	//	to a value pair and a scaling factor pair
	//
	[[nodiscard]] constexpr xy<int> dip(const xy<int> size, const xy<f> dip) noexcept
	{
		return {
			int(f(size.x) * dip.x),
			int(f(size.y) * dip.y)
		};
	}

	//
	//	Registers any windows API class and fills missing details of the
	//	WNDCLASSEXW data struct if any.
	//	The return value shall not be ignored.
	//
	[[nodiscard]] ATOM registerClasses(WNDCLASSEXW & wcex) noexcept;
	//
	//	Asks user info with a given message and window title in a
	//	dialog-box-like window
	//	Information from the user is returned as std::wstring
	//
	[[nodiscard]] std::wstring askInfo(std::wstring_view message, std::wstring_view title) noexcept;

	namespace utf
	{
		//
		//	Converts UTF-8 string to UTF-16 (Unicode) string
		//
		[[nodiscard]] std::wstring conv(std::string_view str);
		//
		//	Converts UTF-16 (Unicode) string to UTF-8 string
		//
		[[nodiscard]] std::string conv(std::wstring_view wstr);
	}
}
