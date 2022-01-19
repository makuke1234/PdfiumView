#pragma once
#include <utility>

namespace pdfv::concepts
{
	template<typename T, typename U>
	concept integral_both = std::is_integral_v<T> && std::is_integral_v<U>;

	template<typename T, typename U>
	concept floating_both = std::is_floating_point_v<T> && std::is_floating_point_v<U>;

	template<typename T, typename U>
	concept integral_and_floating = std::is_integral_v<T> && std::is_floating_point_v<U>;

	template<typename T, typename U>
	concept integral_or_floating_both = integral_both<T, U> || floating_both<T, U>;

	template<typename T, typename U>
	concept trivial_both = std::is_trivial_v<T> && std::is_trivial_v<U>;

	template<typename T, typename U>
	concept nontrivial_both = !std::is_trivial_v<T> && !std::is_trivial_v<U>;

	template<typename T, typename U>
	concept different_trivial = trivial_both<T, U> && (integral_and_floating<T, U> || integral_and_floating<U, T>);

	template<typename T, typename U>
	concept different_atleastone_nontrivial = !trivial_both<T, U> && !std::is_same_v<T, U>;
	
	template<typename T, typename U>
	concept differecnt_nontrivial_both = nontrivial_both<T, U> && !std::is_same_v<T, U>;

}
