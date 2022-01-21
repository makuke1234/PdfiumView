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
	#include "debug.hpp"
	#define DEBUGPRINT(...) pdfv::debug::printf(__VA_ARGS__)

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
		/**
		 * @brief Get the rectangle of client area
		 * 
		 * @param hwnd Window handle
		 * @param def Default rectangle, empty rectangle by default
		 * @return RECT On success client area rectangle, on failure default rectangle
		 */
		[[nodiscard]] RECT getCliR(HWND hwnd, RECT def = {}) noexcept;
		/**
		 * @brief Get the rectangle of window area
		 * 
		 * @param hwnd Window handle
		 * @param def Default rectangle, empty rectangle by default
		 * @return RECT On success window area rectangle, on failure default rectangle
		 */
		[[nodiscard]] RECT getWinR(HWND hwnd, RECT def = {}) noexcept;

		/**
		 * @brief Alias of getCliR
		 * 
		 */
		constexpr auto getClientRect = getCliR;
		/**
		 * @brief Alias of getWinR
		 * 
		 */
		constexpr auto getWindowRect = getWinR;

		/**
		 * @brief Get window text
		 * 
		 * @param hwnd Window handle
		 * @param def Default string, empty string by default
		 * @return std::wstring Window text, up to 2048 characters
		 */
		[[nodiscard]] std::wstring getWinText(HWND hwnd, const std::wstring & def = {});

		/**
		 * @brief Alias of getWinText
		 * 
		 */
		constexpr auto getWindowText = getWinText;

		/**
		 * @brief Set window font
		 * 
		 * @param hwnd Window font
		 * @param hfont Font handle
		 * @param redraw If true, window will be redrawn, false by default
		 */
		void setFont(HWND hwnd, HFONT hfont, bool redraw = false) noexcept;

		/**
		 * @brief Move window
		 * 
		 * @param hwnd Window handle
		 * @param x Horizontal position of window
		 * @param y Vertical position of window
		 * @param redraw If ture, window will be redrawn, false by default
		 * @return true Success
		 * @return false Failure
		 */
		bool moveWin(HWND hwnd, int x, int y, bool redraw = false) noexcept;
		/**
		 * @brief Move window
		 * 
		 * @param hwnd Window handle
		 * @param rect New window rectangle
		 * @param redraw If ture, window will be redrawn, false by default
		 * @return true Success
		 * @return false Failure
		 */
		bool moveWin(HWND hwnd, RECT rect, bool redraw = false) noexcept;

		/**
		 * @brief Get cursor position relative to the screen
		 * 
		 * @param def Default position, 0; 0 by default
		 * @return POINT On success cursor position, default position on failure
		 */
		[[nodiscard]] POINT getCur(POINT def = {}) noexcept;
		/**
		 * @brief Get cursor position relative to the window
		 * 
		 * @param hwnd Window handle
		 * @param def Default position, 0; 0 by default
		 * @return POINT On success cursor position, default position on failure
		 */
		[[nodiscard]] POINT getCur(HWND hwnd, POINT def = {}) noexcept;

		/**
		 * @brief Redraws window
		 * 
		 * @param hwnd Window handle
		 * @param erase Flag, determining whether the window should be erased, false by default
		 * @return true Success
		 * @return false Failure
		 */
		bool redraw(HWND hwnd, bool erase = false) noexcept;

		/**
		 * @brief Reorders windows by Z-position
		 * 
		 * @param hwnd Window handle of window on top
		 * @param hbefore Window handle of window coming before hwnd
		 * @param redraw If true, hwnd bill be redrawn, false by default
		 * @return true Success
		 * @return false Failure
		 */
		bool reorder(HWND hwnd, HWND hbefore, bool redraw = false) noexcept;
		
		/**
		 * @brief Resizes window
		 * 
		 * @param hwnd Window handle
		 * @param x Horizontal size
		 * @param y Vertical size
		 * @param redraw If true, hwnd bill be redrawn, false by default
		 * @return true Success
		 * @return false Failure
		 */
		bool resize(HWND hwnd, int x, int y, bool redraw = false) noexcept;

		/**
		 * @brief Safe wrapper for GDI types
		 * 
		 * @tparam T Any GDI type
		 */
		template<typename T>
		struct GDI
		{
			T obj{ nullptr };

			GDI() = default;
			constexpr GDI(const T & value) noexcept
				: obj(value)
			{
			}
			constexpr GDI(const GDI & other) noexcept
				: obj(other.obj)
			{
			}
			constexpr GDI(GDI && other) noexcept
				: obj(other.obj)
			{
				other.obj = nullptr;
			}
			GDI & operator=(const GDI & other) noexcept
			{
				this->~GDI();
				this->obj = other.obj;
				return *this;
			}
			GDI & operator=(GDI && other) noexcept
			{
				this->~GDI();
				this->obj = other.obj;
				other.obj = nullptr;
				return *this;
			}
			~GDI() noexcept
			{
				if (this->obj != nullptr)
				{
					auto temp{ this->obj };
					this->obj = nullptr;
					::DeleteObject(temp);
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

	extern std::function<void(wchar_t **)> getArgsFree;
	/**
	 * @brief Get wide-stringed argument vector from command line wide-string
	 * 
	 * @param cmdLine Command line wide-string
	 * @param argc Reference to argument vector length, will be valued in function
	 * @return std::unique_ptr<wchar_t *, decltype(pdfv::getArgsFree)> Argument vector
	 */
	[[nodiscard]] std::unique_ptr<wchar_t *, decltype(pdfv::getArgsFree)> getArgs(LPWSTR cmdLine, int & argc) noexcept;
	
	/**
	 * @brief Initialise common controls
	 * 
	 * @return true Success
	 * @return false Failure
	 */
	[[nodiscard]] bool initCC() noexcept;

	/**
	 * @brief Alias of initCC
	 * 
	 */
	constexpr auto initCommontControls = initCC;

	/**
	 * @brief Data structure that represents x, y coordinate pair of a point, provides
	 * modern operator overloading for operations regarding coordinate pair
	 * 
	 * @tparam T Type of coordinate, int by default
	 */
	template<typename T = int>
	struct xy
	{
		T x{}, y{};

		xy() noexcept = default;
		constexpr xy(T x_, T y_) noexcept
			: x(x_), y(y_)
		{
		}
		/**
		 * @brief Constructs value pair from RECT rectangle, requires value type T to be constructible with RECT values,
		 * calculates rectangle size
		 * 
		 */
		constexpr xy(RECT r) noexcept requires std::is_constructible_v<T, decltype(RECT().left)>
			: x(T(r.right - r.left)), y(T(r.bottom - r.top))
		{
		}
		template<typename U>
		constexpr xy(const xy<U> & other) noexcept requires std::is_convertible_v<U, T> && concepts::integral_or_floating_both<T, U>
			: x(T(other.x)), y(T(other.y))
		{
		}
		template<typename U>
		explicit constexpr xy(const xy<U> & other) noexcept requires std::is_convertible_v<U, T> && (!concepts::integral_or_floating_both<T, U>)
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
		/**
		 * @brief Swaps current object with another object
		 * 
		 * @param other The other object to swap with
		 */
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
		/**
		 * @param rhs Right hand side
		 * @return true Value pairs are equal
		 */
		[[nodiscard]] constexpr bool operator==(const xy & rhs) const noexcept
		{
			return (x == rhs.x) && (y == rhs.y);
		}
		/**
		 * @param rhs Right hand side
		 * @return true Value pairs are not equal
		 */
		[[nodiscard]] constexpr bool operator!=(const xy & rhs) const noexcept
		{
			return !this->operator==(rhs);
		}
		/**
		 * @param rhs Right hand side
		 * @return true Left hand side is smaller than right hand side
		 */
		[[nodiscard]] constexpr bool operator< (const xy & rhs) const noexcept
		{
			return (x < rhs.x) && (y < rhs.y);
		}
		/**
		 * @param rhs Right hand side
		 * @return true Left hand side is smaller than or equal to right hand side
		 */
		[[nodiscard]] constexpr bool operator<=(const xy & rhs) const noexcept
		{
			return (x <= rhs.x) && (y <= rhs.y);
		}
		/**
		 * @param rhs Right hand side
		 * @return true Left hand side is larger than right hand side
		 */
		[[nodiscard]] constexpr bool operator> (const xy & rhs) const noexcept
		{
			return (x > rhs.x) && (y > rhs.y);
		}
		/**
		 * @param rhs Right hand side
		 * @return true Left hand side is larger than or equal to right hand side
		 */
		[[nodiscard]] constexpr bool operator>=(const xy & rhs) const noexcept
		{
			return (x >= rhs.x) && (y >= rhs.y);
		}

		/**
		 * @brief Adds two pairs' values together, forms a new pair
		 * 
		 * @param rhs Right hand side
		 * @return xy New pair
		 */
		[[nodiscard]] constexpr xy operator+ (const xy & rhs) const noexcept
		{
			return { x + rhs.x, y + rhs.y };
		}
		/**
		 * @brief Subtracts right hand side pair's values from left hand side pair's values, forms a new pair
		 * 
		 * @param rhs Right hand side
		 * @return xy New pair
		 */
		[[nodiscard]] constexpr xy operator- (const xy & rhs) const noexcept
		{
			return { x - rhs.x, y - rhs.y };
		}
		/**
		 * @brief Multiplies two pairs' values together, forms a new pair
		 * 
		 * @param rhs Right hand side
		 * @return xy New pair
		 */
		[[nodiscard]] constexpr xy operator* (const xy & rhs) const noexcept
		{
			return { x * rhs.x, y * rhs.y };
		}
		/**
		 * @brief Divides right hand side pair's values from left hand side pair's values, forms a new pair
		 * 
		 * @param rhs Right hand side
		 * @return xy New pair
		 */
		[[nodiscard]] constexpr xy operator/ (const xy & rhs) const noexcept
		{
			return { x / rhs.x, y / rhs.y };
		}

		/**
		 * @brief Adds a value to both members of the value pair, forms a new pair
		 * 
		 * @param rhs Right hand side
		 * @return xy New pair
		 */
		[[nodiscard]] constexpr xy operator+ (const T rhs) const noexcept
		{
			return { x + rhs, y + rhs };
		}
		/**
		 * @brief Subtracts a value from both members of the value pair, forms a new pair
		 * 
		 * @param rhs Right hand side
		 * @return xy New pair
		 */
		[[nodiscard]] constexpr xy operator- (const T rhs) const noexcept
		{
			return { x - rhs, y - rhs };
		}
		/**
		 * @brief Multiplies a value with both members of the value pair, forms a new pair
		 * 
		 * @param rhs Right hand side
		 * @return xy New pair
		 */
		[[nodiscard]] constexpr xy operator* (const T rhs) const noexcept
		{
			return { x * rhs, y * rhs };
		}
		/**
		 * @brief Divides a value from both members of the value pair, forms a new pair
		 * 
		 * @param rhs Right hand side
		 * @return xy New pair
		 */
		[[nodiscard]] constexpr xy operator/ (const T rhs) const noexcept
		{
			return { x / rhs, y / rhs };
		}
		
		/**
		 * @brief Adds both members of the other value pair to the current value pair
		 * 
		 * @param rhs Right hand side
		 * @return xy&
		 */
		constexpr xy & operator+=(const xy & rhs) noexcept
		{
			x += rhs.x;
			y += rhs.y;
			return *this;
		}
		/**
		 * @brief Subtracts both members of the other value pair from the current value pair
		 * 
		 * @param rhs Right hand side
		 * @return xy&
		 */
		constexpr xy & operator-=(const xy & rhs) noexcept
		{
			x -= rhs.x;
			y -= rhs.y;
			return *this;
		}
		/**
		 * @brief Multiplies both members of the other value pair with the current value pair
		 * 
		 * @param rhs Right hand side
		 * @return constexpr xy& 
		 */
		constexpr xy & operator*=(const xy & rhs) noexcept
		{
			x *= rhs.x;
			y *= rhs.y;
			return *this;
		}
		/**
		 * @brief Divides both members of the other value pair from the current value pair
		 * 
		 * @param rhs Right hand side
		 * @return constexpr xy& 
		 */
		constexpr xy & operator/=(const xy & rhs) noexcept
		{
			x /= rhs.x;
			y /= rhs.y;
			return *this;
		}
		/**
		 * @brief Bitshifts the current value pair members to left with other value pair members
		 * 
		 * @param rhs Right hand side
		 * @return constexpr xy& 
		 */
		constexpr xy & operator<<=(const xy & rhs) noexcept requires std::is_integral_v<T>
		{
			x <<= rhs.x;
			y <<= rhs.y;
			return *this;
		}
		/**
		 * @brief Bitshifts the current value pair members to right with other value pair members
		 * 
		 * @param rhs Right hand side
		 * @return constexpr xy& 
		 */
		constexpr xy & operator>>=(const xy & rhs) noexcept requires std::is_integral_v<T>
		{
			x >>= rhs.x;
			y >>= rhs.y;
			return *this;
		}


		/**
		 * @brief Adds a value to both members of the value pair
		 * 
		 * @param rhs Right hand side
		 * @return xy&
		 */
		constexpr xy & operator+=(const T & rhs) noexcept
		{
			x += rhs;
			y += rhs;
			return *this;
		}
		/**
		 * @brief Subtracts a value from both members of the value pair
		 * 
		 * @param rhs Right hand side
		 * @return xy&
		 */
		constexpr xy & operator-=(const T & rhs) noexcept
		{
			x -= rhs;
			y -= rhs;
			return *this;
		}
		/**
		 * @brief Multiplies a value with both members of the value pair
		 * 
		 * @param rhs Right hand side
		 * @return xy&
		 */
		constexpr xy & operator*=(const T & rhs) noexcept
		{
			x *= rhs;
			y *= rhs;
			return *this;
		}
		/**
		 * @brief Divides a value from both members of the value pair
		 * 
		 * @param rhs Right hand side
		 * @return xy&
		 */
		constexpr xy & operator/=(const T & rhs) noexcept
		{
			x /= rhs;
			y /= rhs;
			return *this;
		}
		/**
		 * @brief Bitshifts both members of the value pair to left by a integral value
		 * 
		 * @param rhs Right hand side
		 * @return xy&
		 */
		constexpr xy & operator<<=(const T & rhs) noexcept requires std::is_integral_v<T>
		{
			x <<= rhs;
			y <<= rhs;
			return *this;
		}
		/**
		 * @brief Bitshifts both members of the value pair to right by a integral value
		 * 
		 * @param rhs Right hand side
		 * @return xy&
		 */
		constexpr xy & operator>>=(const T & rhs) noexcept requires std::is_integral_v<T>
		{
			x >>= rhs;
			y >>= rhs;
			return *this;
		}
	};

	/**
	 * @brief Makes xy<> coordinate pair from RECT, calculates size of rectangle
	 * 
	 * @param r Rectangle object
	 * @return xy<decltype(RECT().left)> 
	 */
	[[nodiscard]] constexpr xy<decltype(RECT().left)> make_xy(RECT r) noexcept
	{
		return r;
	}
	
	extern xy<f> dpi;
	
	/**
	 * @brief Calculates correct screen coordinate using DI pixel and DPI, performs rounding
	 * 
	 * @param size DI pixel
	 * @param dpi DPI scaling factor
	 * @return int Screen coordinate
	 */
	[[nodiscard]] constexpr int dip(const int size, const f dpi) noexcept
	{
		return int(f(size) * dpi + 0.5f);
	}
	/**
	 * @brief Calculates correct screen coordinate pair using DI pixel pair and DPI pair, performs rounding
	 * 
	 * @param size DI pixel pair
	 * @param dpi DPI scaling factor pair
	 * @return int Screen coordinate pair
	 */
	[[nodiscard]] constexpr xy<int> dip(const xy<int> size, const xy<f> dpi) noexcept
	{
		return xy<int>(xy<f>(size) * dpi + 0.5f);
	}

	/**
	 * @brief Calculates corrent screen font size using DI font size in pt and DPI, performs rounding
	 * 
	 * @param size Font size size in points (pt)
	 * @param dpi DPI scaling factor
	 * @return int Screen coordinate 
	 */
	[[nodiscard]] constexpr int dipfont(const int size, const f dpi) noexcept
	{
		return -int(f(size) * dpi * 96.0f / 72.0f + 0.5f);
	}

	/**
	 * @brief Register any Win32 API class and fills missing fields of WNDCLASSEXW data structure
	 * if any
	 * 
	 * @param wcex Reference to window class extended structure
	 * @return ATOM Non-zero on success
	 */
	[[nodiscard]] ATOM registerClasses(WNDCLASSEXW & wcex) noexcept;
	/**
	 * @brief Asks user info with a given message and window title in a dialog-like window
	 * 
	 * @param message Message to user
	 * @param title Window title
	 * @return std::wstring Information string the user entered
	 */
	[[nodiscard]] std::wstring askInfo(std::wstring_view message, std::wstring_view title) noexcept;

	namespace utf
	{
		/**
		 * @brief Converts UTF-8 string to UTF-16 string
		 * 
		 * @param str UTF-8 string
		 * @return std::wstring UTF-16 string
		 */
		[[nodiscard]] std::wstring conv(std::string_view str);
		/**
		 * @brief Converts UTF-16 string to UTF-8 string
		 * 
		 * @param wstr UTF-16 string
		 * @return std::string UTF-8 string
		 */
		[[nodiscard]] std::string conv(std::wstring_view wstr);
	}
}
