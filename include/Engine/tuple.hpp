#pragma once

/**
 * @file
 * Utilities for working with tuples, variadic arguments, and parameter packs.
 */


namespace Engine {
	/**
	 * Empty class.
	 * Used in places where `void` is invalid.
	 */
	class Nothing {};

	/**
	 * Calls a callable with the given arguments and returns the result.
	 * If the result is `void`, return a Nothing object instead.
	 */
	template<class Func, class... Args>
	ENGINE_INLINE decltype(auto) resultOrNothing(Func&& func, Args&&... args) {
		if constexpr (std::same_as<void, decltype(func(std::forward<Args>(args)...))>) {
			func(std::forward<Args>(args)...);
			return Nothing{};
		} else {
			return func(std::forward<Args>(args)...);
		}
	}

	/**
	 * Calls a callable once with each tuple element.
	 * @param tuple The tuple whos elements are passed to the callable.
	 * @param func The callable to call with each tuple element.
	 * @return A tuple of all results of calling the callable.
	 */
	template<class Tuple, class Func, auto... Is>
	ENGINE_INLINE decltype(auto) forEach(Tuple&& tuple, Func&& func, std::index_sequence<Is...>) {
		// It is important that we use braced initialization `Ret{}` as opposed to `Ret()`
		// here to ensure evaluation order.
		// See: [dcl.init.list/4]: https://eel.is/c++draft/dcl.init.list#4
		using Ret = std::tuple<decltype(resultOrNothing(func, std::get<Is>(std::forward<Tuple>(tuple))))...>;
		return Ret{resultOrNothing(func, std::get<Is>(std::forward<Tuple>(tuple)))...};
	}

	/** @see forEach */
	template<class Tuple, class Func>
	ENGINE_INLINE decltype(auto) forEach(Tuple&& tuple, Func&& func) {
		return forEach(std::forward<Tuple>(tuple), std::forward<Func>(func), std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuple>>>{});
	}

	/**
	 * Calls a callable once with each tuple element, in reverse order.
	 * @param tuple The tuple whos elements are passed to the callable.
	 * @param func The callable to call with each tuple element.
	 * @return A tuple of all results of calling the callable.
	 * @see forEach
	 */
	template<class Tuple, class Func, auto... Is>
	ENGINE_INLINE decltype(auto) forEachReverse(Tuple&& tuple, Func&& func, std::index_sequence<Is...>) {
		// It is important that we use braced initialization `Ret{}` as opposed to `Ret()`
		// here to ensure evaluation order.
		// See: [dcl.init.list/4]: https://eel.is/c++draft/dcl.init.list#4
		using Ret = std::tuple<decltype(resultOrNothing(func, std::get<Is>(std::forward<Tuple>(tuple))))...>;
		return Ret{resultOrNothing(func, std::get<sizeof...(Is) - Is - 1>(std::forward<Tuple>(tuple)))...};
	}

	/** @see forEachReverse */
	template<class Tuple, class Func>
	ENGINE_INLINE decltype(auto) forEachReverse(Tuple&& tuple, Func&& func) {
		return forEachReverse(std::forward<Tuple>(tuple), std::forward<Func>(func), std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuple>>>{});
	}

	namespace Detail {
		template<class Tuple, class Func>
		class WithTypeAt;

		template<template<class...> class Tuple, class... Ts, class Func>
		class WithTypeAt<Tuple<Ts...>, Func> {
			public:
				static_assert(sizeof...(Ts) > 0, "Expected a tuple with one or more types.");
				decltype(auto) call(size_t idx, Func&& func) {
					// All overloads must return the same type.
					using Return = decltype(std::declval<Func>().template operator()<std::tuple_element_t<0, Tuple<Ts...>>>());
					using MemberFunc = Return(Func::*)(void) const;
					constexpr MemberFunc members[]{ &Func::template operator()<Ts>... };
					ENGINE_DEBUG_ASSERT(idx < std::size(members), "Attempting to index an invalid function.");
					return (func.*members[idx])();
				}
		};
	}

	// TODO: a version that takes and passes a tuple ref value as param would probably also be useful.
	/**
	 * Call the given function with the @p idx type from the given tuple.
	 * Note that this is only the type from the tuple. It does not pass any value to the
	 * function.
	 */
	template<class Tuple, class Func>
	decltype(auto) withTypeAt(const size_t idx, Func&& func) {
		return Detail::WithTypeAt<Tuple, Func>{}.call(idx, std::forward<Func>(func));
	}

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
		using Type = std::tuple<>;
	};
	
	template<class Check, class T>
	requires requires { (std::declval<Check>()(std::declval<T>())); }
	struct MemberTypeIfExists<Check, T> {
		using Type = std::tuple<decltype(std::declval<Check>()(std::declval<T>()))>;
	};

	template<class Check, class T>
	using MemberTypeIfExists_t = MemberTypeIfExists<Check, T>::Type;

	/**
	 * Joins two type lists together.
	 */
	template<class Tuple1 = void, class Tuple2 = void, class... TupleN>
	struct TupleConcat;
	
	template<>
	struct TupleConcat<void, void> {
		using Type = std::tuple<>;
	};

	template<class Tuple1>
	struct TupleConcat<Tuple1, void> {
		using Type = Tuple1;
	};

	template<template<class...> class Tuple, class... Args1, class... Args2>
	struct TupleConcat<Tuple<Args1...>, Tuple<Args2...>> {
		using Type = Tuple<Args1..., Args2...>;
	};

	template<template<class...> class Tuple, class... Args1, class... Args2, class Tuple3, class... TupleN>
	struct TupleConcat<Tuple<Args1...>, Tuple<Args2...>, Tuple3, TupleN...>
		: TupleConcat<Tuple<Args1..., Args2...>, Tuple3, TupleN...> {
	};

	template<class... TupleN>
	using TupleConcat_t = TupleConcat<TupleN...>::Type;

	
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
			MemberTypeIfExists_t<Check, T>,
			typename TupleJoinMembersTypesIfExists<Check, Tuple<Args...>>::Type // Cannot use the _t alias in the definition. Must use the dependant type name.
		>
	{};

	template<class Check, class Tuple>
	using TupleJoinMembersTypesIfExists_t = TupleJoinMembersTypesIfExists<Check, Tuple>::Type;


	namespace Detail {
		template<class Set>
		struct MakeAll;

		template<template<class...> class Set, class... Ts>
		struct MakeAll<Set<Ts...>> {
			template<class... Args>
			decltype(auto) call(Args&... args) {
				return Set(Ts(args...)...);
			}
		};
	}
	
	/**
	 * Construct a tuple we the same constructor arguments for each tuple element.
	 */
	template<class Set, class... Args>
	decltype(auto) makeAll(Args&... args) {
		return Detail::MakeAll<Set>{}.call(args...);
	}
}
