#pragma once

// RHH
#include <robin_hood.h>

// STD
#include <type_traits>

// TODO: Doc
namespace Engine {
	template<class T, class SFINAE = void>
	struct Hash : robin_hood::hash<T> {};

	// TODO: Doc - specialization for enums
	template<class T>
	struct Hash<T, std::enable_if_t<std::is_enum_v<T>>> : Hash<std::underlying_type_t<T>> {
		size_t operator()(const T& val) const {
			using U =  std::underlying_type_t<T>;
			return Hash<U>::operator()(static_cast<U>(val));
		}
	};

	// TODO: split
	inline void hashCombine(size_t& seed, size_t value) {
		seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
}