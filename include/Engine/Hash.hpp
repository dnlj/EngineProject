#pragma once

// RHH
#include <robin_hood.h>


namespace Engine {
	template<class T>
	class Hash : public robin_hood::hash<T> {};

	// TODO: split
	inline void hashCombine(size_t& seed, size_t value) {
		seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
}
