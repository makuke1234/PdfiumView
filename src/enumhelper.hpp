#pragma once

#include "concepts.hpp"

namespace pdfv
{
	template<concepts::enum_concept Enum>
	struct Enumhelper_enabler;

	template<concepts::enum_concept Enum>
	using UnderT = std::underlying_type_t<Enum>;

	template<concepts::enum_concept Enum>
	[[nodiscard]] constexpr Enum operator|(Enum lhs, Enum rhs) noexcept requires std::is_constructible_v<Enumhelper_enabler<Enum>>
	{
		return Enum{ UnderT<Enum>(lhs) | UnderT<Enum>(rhs) };
	}
	template<concepts::enum_concept Enum>
	[[nodiscard]] constexpr Enum operator&(Enum lhs, Enum rhs) noexcept requires std::is_constructible_v<Enumhelper_enabler<Enum>>
	{
		return Enum{ UnderT<Enum>(lhs) & UnderT<Enum>(rhs) };
	}
	template<concepts::enum_concept Enum>
	[[nodiscard]] constexpr Enum operator^(Enum lhs, Enum rhs) noexcept requires std::is_constructible_v<Enumhelper_enabler<Enum>>
	{
		return Enum{ UnderT<Enum>(lhs) ^ UnderT<Enum>(rhs) };
	}

	template<concepts::enum_concept Enum>
	[[nodiscard]] constexpr bool operator!(Enum lhs) noexcept requires std::is_constructible_v<Enumhelper_enabler<Enum>>
	{
		return !UnderT<Enum>(lhs);
	}
	template<concepts::enum_concept Enum>
	[[nodiscard]] constexpr Enum operator~(Enum lhs) noexcept requires std::is_constructible_v<Enumhelper_enabler<Enum>>
	{
		return Enum{ ~UnderT<Enum>(lhs) };
	}
}
