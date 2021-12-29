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

	/** @see IsAnyOf */
	template<class T, class... Us>
	constexpr inline bool IsAnyOf_v = IsAnyOf<T, Us...>::value;
}
