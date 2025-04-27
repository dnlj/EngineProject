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
}
