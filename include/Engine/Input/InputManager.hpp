#pragma once

// STD
#include <vector>
#include <string>
#include <string_view>
#include <type_traits>
#include <tuple>

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/Input/InputId.hpp>
#include <Engine/Input/Bind.hpp>
#include <Engine/Input/BindId.hpp>
#include <Engine/Input/InputSequence.hpp>


namespace Engine::Input {
	class InputManager {
		public:
			FlatHashMap<InputId, std::vector<BindId>> bindLookup;
			std::vector<Bind> binds;
			
		public:
			/**
			 * Applies an input to all binds.
			 */
			void processInput(const InputState& is);

			/**
			 * Adds a listener for a sequence of inputs.
			 */
			template<class Listener>
			BindId addBind(const InputSequence& inputs, Listener&& listener);
	};
}

#include <Engine/Input/InputManager.ipp>
