// Engine
#include <Engine/Input/InputBindMapping.hpp>

namespace Engine::Input {
	InputBindMapping::InputBindMapping(BindId bid, InputSequence inputs)
		: bid{bid} {

		inputStates.resize(inputs.size());

		for (int i = 0; i < inputs.size(); ++i) {
			inputStates[i].input = std::move(inputs[i]);
		}
	}

	void InputBindMapping::processInput(const InputState& is) {
		active = true;

		for (int i = 0; i < inputStates.size() - 1; ++i) {
			auto& s = inputStates[i];

			if (s.input == is.input) {
				s.state = is.state;
				active = false;
				return; // There should only be one state for each input. No need to check the rest.
			}

			active = active && s.state;
		}

		// If we havent already returned we are dealing with the last input
		auto& last = inputStates.back();
		last.state = is.state;
		active = active && last.state;
	}

	bool InputBindMapping::isActive() const {
		return active;
	}

	BindId InputBindMapping::getBindId() const {
		return bid;
	}
};
