#pragma once

// Engine
#include <Engine/Glue/Glue.hpp>

// Box2D
#include <box2d/b2_math.h>

// GLM
#include <glm/vec2.hpp>


namespace Engine::Glue::_impl {
	template<class From>
	struct as<b2Vec2, From> {
		// Verify assumptions
		static_assert(sizeof(b2Vec2) == 2 * sizeof(float32));
		static_assert(sizeof(b2Vec2) == sizeof(glm::vec2));
		static_assert(std::is_same_v<decltype(b2Vec2::x), float32>);
		static_assert(std::is_same_v<decltype(b2Vec2::y), float32>);
		static_assert(std::is_same_v<decltype(glm::vec2::x), float32>);
		static_assert(std::is_same_v<decltype(glm::vec2::y), float32>);
		static_assert(offsetof(b2Vec2, x) == offsetof(glm::vec2, x));
		static_assert(offsetof(b2Vec2, y) == offsetof(glm::vec2, y));

		static b2Vec2 call(const glm::ivec2& v) {
			return b2Vec2{static_cast<float32>(v.x), static_cast<float32>(v.y)};
		}

		static b2Vec2& call(glm::vec2& v) {
			return reinterpret_cast<b2Vec2&>(v);
		}

		static b2Vec2&& call(glm::vec2&& v) {
			return reinterpret_cast<b2Vec2&&>(v);
		}
	};
}
