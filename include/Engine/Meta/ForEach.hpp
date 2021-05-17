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
}
