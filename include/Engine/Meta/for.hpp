#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Meta {
	template<class... Ts>
	struct ForEach {
		template<class Func>
		ENGINE_INLINE static void call(Func&& func) {
			(func.operator()<Ts>(), ...);
		}
	};

	template<class Set>
	struct ForEachIn;

	template<template<class...> class Set, class... Ts>
	struct ForEachIn<Set<Ts...>> : ForEach<Ts...> {};

	template<class... Ts>
	struct ForAll {
		template<class Func>
		ENGINE_INLINE constexpr static decltype(auto) call(Func&& func) {
			return std::forward<Func>(func).operator()<Ts...>();
		}
	};

	template<class Set>
	struct ForAllIn;

	template<template<class...> class Set, class... Ts>
	struct ForAllIn<Set<Ts...>> : ForAll<Ts...> {};

	/**
	 * Calls a callable with the template parameters from the type @p Set.
	 */
	template<class Set, class Func>
	ENGINE_INLINE constexpr decltype(auto) forAll(Func&& func) {
		return ForAllIn<Set>::call(std::forward<Func>(func));
	}
}
