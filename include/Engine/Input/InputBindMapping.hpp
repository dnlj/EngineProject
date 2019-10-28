#pragma once

// STD
#include <array>

// Engine
#include <Engine/FlatHashMap.hpp>
#include <Engine/Input/InputId.hpp>
#include <Engine/Input/InputSequence.hpp>
#include <Engine/Input/InputState.hpp>
#include <Engine/Input/BindId.hpp>


// TODO: Doc
namespace Engine::Input {
	class InputBindMapping {
		public:
			InputBindMapping(BindId bid, InputSequence inputs);
			void processInput(const InputState& is);
			bool isActive() const;
			BindId getBindId() const;

		private:
			bool active = false;
			const BindId bid;
			// TODO: Do we really want to use InputState here? While it works, it seems semantically incorrect.
			InputStateSequence inputStates;
	};
};
