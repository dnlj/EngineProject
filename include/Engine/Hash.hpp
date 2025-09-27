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

	template<class T>
	[[nodiscard]] ENGINE_INLINE inline size_t hash(const T& val) {
		return Hash<T>()(val);
	}
	
	[[nodiscard]] ENGINE_INLINE inline size_t hashBytes(const void* data, size_t len) {
		return robin_hood::hash_bytes(data, len);
	}

	ENGINE_INLINE constexpr inline void hashCombine(size_t& seed, size_t value) {
		seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	// TODO: should this be in Engine::Glue or similar?
	template<>
	struct Hash<glm::ivec2> {
		[[nodiscard]] ENGINE_INLINE size_t operator()(const glm::ivec2& val) const {
			static_assert(sizeof(size_t) == sizeof(val));
			return robin_hood::hash_int(std::bit_cast<size_t>(val));
		}
	};

	template<>
	struct Hash<glm::i64vec2> {
		[[nodiscard]] ENGINE_INLINE size_t operator()(const glm::i64vec2& val) const {
			size_t result = val.x;
			hashCombine(result, val.y);
			return result;
		}
	};

	// TODO: where to put this?
	template<class C>
	struct Hash<std::basic_string<C>> {
		using is_transparent = void;

		[[nodiscard]] ENGINE_INLINE size_t operator()(const std::basic_string_view<C>& val) const noexcept {
			return hashBytes(std::data(val), sizeof(C) * std::size(val));
		}

		[[nodiscard]] ENGINE_INLINE size_t operator()(const std::basic_string<C>& val) const noexcept {
			return (*this)(std::basic_string_view<C>(val));
		}

		[[nodiscard]] ENGINE_INLINE size_t operator()(const C* val) const noexcept {
			return (*this)(std::basic_string_view<C>(val));
		}
	};
}
