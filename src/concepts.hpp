#pragma once
#include <utility>

namespace pdfv::concepts
{
	template<typename T, typename U>
	concept integral = std::is_integral_v<T> && std::is_integral_v<U>;

	template<typename T, typename U>
	concept floating = std::is_floating_point_v<T> && std::is_floating_point_v<U>;

	template<typename T, typename U>
	concept integral_and_floating = std::is_integral_v<T> && std::is_floating_point_v<U>;

	template<typename T, typename U>
	concept floating_or_integral = integral<T, U> || floating<T, U>;

	template<typename T, typename U>
	concept different_trivial = integral_and_floating<T, U> || integral_and_floating<U, T>;

}