#pragma once

// RHH
#include <robin_hood.h>

// STD
#include <type_traits>

// GLM
#include <glm/glm.hpp>

// TODO: Doc
namespace Engine {
	template<class T, class SFINAE = void>
	struct Hash : robin_hood::hash<T> {};

	// TODO: Doc - specialization for enums
	template<class T>
	struct Hash<T, std::enable_if_t<std::is_enum_v<T>>> : Hash<std::underlying_type_t<T>> {
		[[nodiscard]]
		size_t operator()(const T& val) const {
			using U =  std::underlying_type_t<T>;
			return Hash<U>::operator()(static_cast<U>(val));
		}
	};

	template<class T>
	[[nodiscard]]
	inline size_t hash(const T& val) {
		return Hash<T>()(val);
	}
	
	[[nodiscard]]
	inline size_t hashBytes(const void* data, size_t len) {
		return robin_hood::hash_bytes(data, len);
	}

	inline void hashCombine(size_t& seed, size_t value) {
		seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	// TODO: should this be in Engine::Glue or similar?
	template<>
	struct Hash<glm::ivec2> {
		[[nodiscard]]
		size_t operator()(const glm::ivec2& val) const {
			static_assert(sizeof(size_t) == sizeof(val));
			return *reinterpret_cast<const size_t*>(&val);
		}
	};

	// TODO: where to put this?
	template<class C>
	struct Hash<std::basic_string<C>> {
		[[nodiscard]]
		size_t operator()(const std::basic_string<C>& val) const noexcept {
			return hashBytes(std::data(val), std::size(val));
		}

		[[nodiscard]]
		size_t operator()(const std::basic_string_view<C>& val) const noexcept {
			return hashBytes(std::data(val), std::size(val));
		}

		[[nodiscard]]
		size_t operator()(const C* val) const noexcept {
			return (*this)(std::basic_string_view<C>(val));
		}
	};
}
