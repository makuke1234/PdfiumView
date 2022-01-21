#pragma once
#include <utility>

namespace pdfv::concepts
{
	/**
	 * @brief Both type have to be integral
	 * 
	 * @tparam T 
	 * @tparam U 
	 */
	template<typename T, typename U>
	concept integral_both = std::is_integral_v<T> && std::is_integral_v<U>;

	/**
	 * @brief Both types have to floating point
	 * 
	 * @tparam T 
	 * @tparam U 
	 */
	template<typename T, typename U>
	concept floating_both = std::is_floating_point_v<T> && std::is_floating_point_v<U>;

	/**
	 * @brief First type has to be integral and second type has to be floating point
	 * 
	 * @tparam T 
	 * @tparam U 
	 */
	template<typename T, typename U>
	concept integral_and_floating = std::is_integral_v<T> && std::is_floating_point_v<U>;

	/**
	 * @brief Both types have to be either integral or floating point
	 * 
	 * @tparam T 
	 * @tparam U 
	 */
	template<typename T, typename U>
	concept integral_or_floating_both = integral_both<T, U> || floating_both<T, U>;

	/**
	 * @brief Both types have to be trivial
	 * 
	 * @tparam T 
	 * @tparam U 
	 */
	template<typename T, typename U>
	concept trivial_both = std::is_trivial_v<T> && std::is_trivial_v<U>;

	/**
	 * @brief Both types have to be non-trivial
	 * 
	 * @tparam T 
	 * @tparam U 
	 */
	template<typename T, typename U>
	concept nontrivial_both = !std::is_trivial_v<T> && !std::is_trivial_v<U>;

	/**
	 * @brief Both types have to be trivial, yet one has to be of integral type and
	 * the other has to be of floating point type
	 * 
	 * @tparam T
	 * @tparam U
	 */
	template<typename T, typename U>
	concept different_trivial = trivial_both<T, U> && (integral_and_floating<T, U> || integral_and_floating<U, T>);

	/**
	 * @brief Types have to be different, at least one type has to be non-trivial
	 * 
	 * @tparam T 
	 * @tparam U 
	 */
	template<typename T, typename U>
	concept different_atleastone_nontrivial = !trivial_both<T, U> && !std::is_same_v<T, U>;
	
	/**
	 * @brief Types have to be different, both types have to be non-trivial
	 * 
	 * @tparam T 
	 * @tparam U 
	 */
	template<typename T, typename U>
	concept differecnt_nontrivial_both = nontrivial_both<T, U> && !std::is_same_v<T, U>;

}
