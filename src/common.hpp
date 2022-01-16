#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <commctrl.h>
#include <fpdfview.h>
#include <cassert>
#include <utility>
#include <cstdint>
#include <string>
#include <string_view>
#include <memory>

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


namespace pdfv {
	// Some type aliases
	using ssize_t = std::intptr_t;
	using uchar = unsigned char;
	using ushort = unsigned short;
	using uint = unsigned int;
	using l = long;
	using ul = unsigned long;
	using ll = long long;
	using ull = unsigned long long;
	using f = float;
	using d = double;
	using ld = long double;
	
	//
	//	Data structure that represents x and y coordinate pair of a point
	//	Provides some modern operator overloading
	//
	template<class T = int>
	class xy {
	public:
		T x{}, y{};

		xy() noexcept = default;
		constexpr xy(T x_, T y_) noexcept
			: x(x_), y(y_)
		{}
		constexpr xy(xy const& other) noexcept
			: x(other.x), y(other.y)
		{}
		constexpr xy(xy&& other) noexcept
			: x(std::move(other.x)), y(std::move(other.y))
		{}
		constexpr xy& operator=(xy const& other) noexcept
		{
			x = other.x;
			y = other.y;
			return *this;
		}
		constexpr xy& operator=(xy&& other) noexcept
		{
			x = std::move(other.x);
			y = std::move(other.y);
			return *this;
		}
		~xy() noexcept = default;
		//
		//	Swaps current object with another class object
		//
		constexpr void swap(xy& other) noexcept
		{
			xy temp{ std::move(other) };
			other = std::move(*this);
			*this = std::move(temp);
		}

		//
		//	Tells if values of value pairs are close enough (both)
		//	Precision is determined by the constant epsilon (second argument)
		//
		[[nodiscard]] constexpr bool isclose(xy const& rhs, const T epsilon) noexcept
		{
			return (std::abs(x - rhs.x) <= epsilon) && (std::abs(y - rhs.y) <= epsilon);
		}
		//
		//	Tells if value pairs are equal to each other
		//
		[[nodiscard]] constexpr bool operator==(xy const& rhs) noexcept
		{
			return (x == rhs.x) && (y == rhs.y);
		}
		//
		//	Tells if value pairs are not equal to each other
		//
		[[nodiscard]] constexpr bool operator!=(xy const& rhs) noexcept
		{
			return (x != rhs.x) || (y != rhs.y);
		}
		//
		//	Tells if both members of current value pair are smaller than
		//	the members of the other value pair 
		//
		[[nodiscard]] constexpr bool operator< (xy const& rhs) noexcept
		{
			return (x < rhs.x) && (y < rhs.y);
		}
		//
		//	Tells if both members of current value pair are smaller than or
		//	equal to the members of the other value pair 
		//
		[[nodiscard]] constexpr bool operator<=(xy const& rhs) noexcept
		{
			return (x <= rhs.x) && (y <= rhs.y);
		}
		//
		//	Tells if both members of current value pair are larger than
		//	the members of the other value pair 
		//
		[[nodiscard]] constexpr bool operator> (xy const& rhs) noexcept
		{
			return (x > rhs.x) && (y > rhs.y);
		}
		//
		//	Tells if both members of current value pair are larger than or
		//	equal to the members of the other value pair 
		//
		[[nodiscard]] constexpr bool operator>=(xy const& rhs) noexcept
		{
			return (x >= rhs.x) && (y >= rhs.y);
		}

		//
		//	Adds up both members of value pairs and makes the answer a new
		//	value pair
		//
		[[nodiscard]] constexpr xy operator+ (xy const& rhs) noexcept
		{
			return { x + rhs.x, y + rhs.y };
		}
		//
		//	Subtracts both members of the other value pair from
		//	current value pair and makes the answer a new value pair
		//
		[[nodiscard]] constexpr xy operator- (xy const& rhs) noexcept
		{
			return { x - rhs.x, y - rhs.y };
		}
		//
		//	Multiplies both members of the other value pair with current
		//	value pair members and makes the answer a new value pair
		//
		[[nodiscard]] constexpr xy operator* (xy const& rhs) noexcept
		{
			return { x * rhs.x, y * rhs.y };
		}
		//
		//	Divides both members of the current value pair with the members
		//	of the other value pair and makes the answer a new value pair
		//
		[[nodiscard]] constexpr xy operator/ (xy const& rhs) noexcept
		{
			return { x / rhs.x, y / rhs.y };
		}
		
		//
		//	Adds both members of the other value pair to the current value pair
		//
		constexpr xy& operator+=(xy const& rhs) noexcept
		{
			x += rhs.x;
			y += rhs.y;
			return *this;
		}
		//
		//	Subtracts both members of the other value pair from the
		//	current value pair
		//
		constexpr xy& operator-=(xy const& rhs) noexcept
		{
			x -= rhs.x;
			y -= rhs.y;
			return *this;
		}
		//
		//	Multiplies the current value pair members
		//	with the other value pair members
		//
		constexpr xy& operator*=(xy const& rhs) noexcept
		{
			x *= rhs.x;
			y *= rhs.y;
			return *this;
		}
		//
		//	Divides the other value pair members
		//	from the current value pair
		//
		constexpr xy& operator/=(xy const& rhs) noexcept
		{
			x /= rhs.x;
			y /= rhs.y;
			return *this;
		}
		//
		//	Bitshifts the current value pair members to left
		//	with the members of the other value pair
		//
		template<typename = typename std::enable_if<std::is_integral_v<T>>>
		constexpr xy& operator<<=(xy const& rhs) noexcept
		{
			x <<= rhs.x;
			y <<= rhs.y;
			return *this;
		}
		//
		//	Bitshift the current value pair members to right
		//	with the members of the other value pair
		//
		template<typename = typename std::enable_if<std::is_integral_v<T>>>
		constexpr xy& operator>>=(xy const& rhs) noexcept
		{
			x >>= rhs.x;
			y >>= rhs.y;
			return *this;
		}


		//
		//	Adds a value to both members of the current value pair
		//
		constexpr xy& operator+=(T const& rhs) noexcept
		{
			x += rhs;
			y += rhs;
			return *this;
		}
		//
		//	Subtracts a value from both members of the current value pair
		//
		constexpr xy& operator-=(T const& rhs) noexcept
		{
			x -= rhs;
			y -= rhs;
			return *this;
		}
		//
		//	Multiplies a value with both members of the current value pair
		//
		constexpr xy& operator*=(T const& rhs) noexcept
		{
			x *= rhs;
			y *= rhs;
			return *this;
		}
		//
		//	Divides a value from both members of the current value pair
		//
		constexpr xy& operator/=(T const& rhs) noexcept
		{
			x /= rhs;
			y /= rhs;
			return *this;
		}
		//
		//	Bitshifts the current value pair members to left
		//	with the given value
		//
		template<typename = typename std::enable_if<std::is_integral_v<T>>>
		constexpr xy& operator<<=(T const& rhs) noexcept
		{
			x <<= rhs;
			y <<= rhs;
			return *this;
		}
		//
		//	Bitshift the current value pair members to right
		//	with the given value
		//
		template<typename = typename std::enable_if<std::is_integral_v<T>>>
		constexpr xy& operator>>=(T const& rhs) noexcept
		{
			x >>= rhs;
			y >>= rhs;
			return *this;
		}
	};
	
	extern xy<float> dpi;
	
	//
	//	Calculates correct screen coordinate according to a given coordinate
	//	and a given scaling factor
	//
	[[nodiscard]] constexpr int DPI(const int size, const float dip) noexcept
	{
		return int(float(size) * dip);
	}
	//
	//	Calcluates correct screen coordinate pair according
	//	to a value pair and a scaling factor pair
	//
	[[nodiscard]] constexpr xy<int> DPI(const xy<int> size, const xy<float> dip) noexcept
	{
		return {
			int(float(size.x) * dip.x),
			int(float(size.y) * dip.y)
		};
	}

	//
	//	Registers any windows API class and fills missing details of the
	//	WNDCLASSEXW data struct if any.
	//	The return value shall not be ignored.
	//
	[[nodiscard]] ATOM RegisterClasses(WNDCLASSEXW& wcex) noexcept;
	//
	//	Asks user info with a given message and window title in a
	//	dialog-box-like window
	//	Information from the user is returned as std::wstring
	//
	[[nodiscard]] std::wstring AskInfo(std::wstring_view message, std::wstring_view title) noexcept;

	//
	//	Converts UTF-8 string to UTF-16 (Unicode) string
	//
	[[nodiscard]] std::wstring Convert(std::string_view input);
	//
	//	Converts UTF-16 (Unicode) string to UTF-8 string
	//
	[[nodiscard]] std::string Convert(std::wstring_view input);
}