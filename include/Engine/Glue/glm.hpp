#pragma once

// STD
#include <tuple>

// Engine
#include <Engine/Glue/Glue.hpp>

// GLM
#include <glm/vec2.hpp>

struct b2Vec2; // Forward decl

namespace Engine::Glue::_impl {
	template<class From>
	struct as<glm::vec2, From> {
		using To = glm::vec2;

		template<class X, class Y>
		static To call(const std::tuple<X, Y>& v) {
			return To{static_cast<float32>(std::get<0>(v)), static_cast<float32>(std::get<1>(v))};
		}

		static const To& call(const b2Vec2& v) {
			return reinterpret_cast<const To&>(v);
		}

		static To& call(b2Vec2& v) {
			return reinterpret_cast<To&>(v);
		}

		static To&& call(b2Vec2&& v) {
			return reinterpret_cast<To&&>(v);
		}
	};
}
