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
}
