#pragma once

// STD
#include <concepts>

namespace Engine {
	/**
	 * Translates from a type to a unique integral suitable for use as an index.
	 * No two inputs values should map to the same output.
	 * Outputs should be mostly sequential.
	 */
	template<class T>
	struct IndexHash {
		int32 operator()(const T& v) const {
			static_assert(false, "IndexHash is not specialized for this type");
		}
	};

	template<std::integral T>
	struct IndexHash<T> {
		T operator()(const T& v) const {
			return v;
		}
	};
}
