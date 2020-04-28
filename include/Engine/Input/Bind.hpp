#pragma once

// STD
#include <array>

// Engine
#include <Engine/FlatHashMap.hpp>
#include <Engine/Input/InputSequence.hpp>
#include <Engine/Input/BindListener.hpp>


namespace Engine::Input {
	class Bind {
		private:
			Value state;
			InputStateSequence inputStates;
			BindListener listener;

		public:
			Bind(InputSequence inputs, BindListener listener);
			void processInput(const InputState& is);
			Value getState() const;
			void notify(Value curr, Value prev) const;

	};
};
