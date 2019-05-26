#pragma once

// STD
#include <array>

// Engine
#include <Engine/FlatHashMap.hpp>
#include <Engine/Input.hpp>
#include <Engine/InputSequence.hpp>
#include <Engine/InputState.hpp>


// TODO: Doc
namespace Engine {
	using BindId = int; // TODO: Move

	class InputBindMapping {
		public:
			InputBindMapping(InputSequence inputs, BindId bid);
			void processInput(const InputState& is);
			bool isActive() const;
			BindId getBindId() const;

		private:
			bool active = false;
			const BindId bid;
			std::array<InputState, std::tuple_size<InputSequence>::value> inputStates;
	};
};
