#pragma once

/**
 * @file
 * Generic type traits.
 */


namespace Engine {
	/**
	 * Forwards the type unless it is an rvalue reference, in which case resolves to a non-reference type.
	 */
	template<class T>
	struct RemoveRValueRef { using type = T; };
	
	/** @see RemoveRValueRef */
	template<class T>
	struct RemoveRValueRef<T&&> { using type = T; };

	/** @see RemoveRValueRef */
	template<class T>
	using RemoveRValueRef_t = typename RemoveRValueRef<T>::type;

	/**
	 * Checks if a type is any of the remaining types.
	 */
	template<class T, class... Us>
	struct IsAnyOf : std::bool_constant<(std::same_as<T, Us> || ...)> {};

	template<class T, class... Us>
	constexpr inline bool IsAnyOf_v = IsAnyOf<T, Us...>::value;
	
	template<class T, class... Us>
	concept AnyOf = IsAnyOf_v<T, Us...>;

	/*
	 * Checks if a type is a character type.
	 * Signed and unsigned char are excluded to avoid issues with (u)int8_t.
	 */
	template<class T>
	struct IsChar : IsAnyOf<std::remove_cvref_t<T>, char, wchar_t, char8_t, char16_t, char32_t> {};

	template<class T>
	constexpr inline bool IsChar_v = IsChar<T>::value;

	template<class T>
	concept AnyChar = IsChar_v<T>;

	template<class T>
	concept AnyNonChar = !AnyChar<T>;

	/**
	 * Check if a type is a floating point or integral.
	 */
	template<class T>
	struct IsNumber : std::disjunction<std::is_integral<T>, std::is_floating_point<T>> {};

	template<class T>
	constexpr inline bool IsNumber_v = IsNumber<T>::value;

	template<class T>
	concept AnyNumber = IsNumber_v<T>;

	/**
	 * Converts an integral or enum type to the appropriate underlying type.
	 */
	template<class T>
	struct AsIntegral;

	template<class T>
	requires std::is_enum_v<T>
	struct AsIntegral<T> {
		using type = std::underlying_type_t<T>;
	};

	template<class T>
	requires std::is_integral_v<T>
	struct AsIntegral<T> {
		using type = T;
	};
}
