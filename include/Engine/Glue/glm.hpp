#pragma once

// STD
#include <tuple>

// Engine
#include <Engine/Glue/Glue.hpp>

// GLM
#include <glm/vec2.hpp>


namespace Engine::Glue::_impl {
	template<class From>
	struct as<glm::vec2, From> {
		using To = glm::vec2;

		template<class X, class Y>
		static To call(const std::tuple<X, Y>& v) {
			return To{static_cast<float32>(std::get<0>(v)), static_cast<float32>(std::get<1>(v))};
		}
	};
}
