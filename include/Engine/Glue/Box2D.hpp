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
		// TODO: add static checks for struct compat here

		static b2Vec2& call(glm::vec2& v) {
			return reinterpret_cast<b2Vec2&>(v);
		}

		static b2Vec2&& call(glm::vec2&& v) {
			return reinterpret_cast<b2Vec2&&>(v);
		}
	};
}
