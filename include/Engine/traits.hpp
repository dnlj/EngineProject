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
	// TODO: Consider using actual value types + decltype to simplify folding
	template<class... Types>
	struct TypeList {};

	/**
	 * Used to check if a type has the given member.
	 * 
	 * Used in conjunction with other trait classes. This is achieved using SFINAE + requires
	 * clauses like can be seen with MemberTypeIfExists. Using the requires in the
	 * decltype(lambda) as well as at the user trait class effectively lets us forward/pass the
	 * concept from the call site to the trait. We then use the return value + decltype to pass
	 * the actual type.
	 */
	#define ENGINE_TRAIT_MEMBER_TYPE_CHECK(Member) decltype([]<class T>(const T&) -> typename T::Member requires requires { typename T::Member; } {})

	/**
	 * Defines either a type list containing the member type if it exists, otherwise an empty type list.
	 * Useful for excluding items from a template type pack.
	 * 
	 * @see ENGINE_TRAIT_MEMBER_TYPE_CHECK
	 */
	template<class Check, class T>
	struct MemberTypeIfExists {
		using Type = TypeList<>;
	};
	
	template<class Check, class T>
	requires requires { (std::declval<Check>()(std::declval<T>())); }
	struct MemberTypeIfExists<Check, T> {
		using Type = TypeList<decltype(std::declval<Check>()(std::declval<T>()))>;
	};

	/**
	 * Joins two type lists together.
	 */
	template<class Tuple1, class Tuple2>
	struct TupleConcat;
	
	template<template<class...> class Tuple, class... Args1, class... Args2>
	struct TupleConcat<Tuple<Args1...>, Tuple<Args2...>> {
		using Type = Tuple<Args1..., Args2...>;
	};

	
	// TODO: It is also possible to write this by defining a operator+ (concat) for
	//       TypeList and then we can use fold expressions on empty types to do the join
	//       inside like: decltype(... + args). That doesn't change this implementation,
	//       but when used elsewhere the fold expressions tend to be more readable than
	//       recursive inheritance like you otherwise need to do. I also wonder if one has
	//       better compile perf than the other. Need to test.
	/**
	 * Concatenates the member types from each tuple type if it exists, otherwise they are excluded.
	 */
	template<class Check, class Tuple>
	struct TupleJoinMembersTypesIfExists;
	
	template<class Check, template<class...> class Tuple>
	struct TupleJoinMembersTypesIfExists<Check, Tuple<>> { using Type = Tuple<>; };
	
	template<class Check, template<class...> class Tuple, class... Args, class T>
	struct TupleJoinMembersTypesIfExists<Check, Tuple<T, Args...>>
		: TupleConcat<
			typename MemberTypeIfExists<Check, T>::Type,
			typename TupleJoinMembersTypesIfExists<Check, Tuple<Args...>>::Type
		>
	{};
}
