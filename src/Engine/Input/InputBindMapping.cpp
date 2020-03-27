// Engine
#include <Engine/Input/InputBindMapping.hpp>

namespace Engine::Input {
	InputBindMapping::InputBindMapping(BindId bid, InputSequence inputs)
		: bid{bid} {

		inputStates.resize(inputs.size());

		for (int i = 0; i < inputs.size(); ++i) {
			inputStates[i].id = std::move(inputs[i]);
		}
	}

	void InputBindMapping::processInput(const InputState& is) {
		ENGINE_DEBUG_ASSERT(!is.id.isAxis(), "Axis input not valid in button mapping.");
		active = true;

		for (int i = 0; i < inputStates.size() - 1; ++i) {
			auto& s = inputStates[i];

			if (s.id == is.id) {
				s.value = is.value;
				active = false;
				return; // There should only be one state for each input. No need to check the rest.
			}

			active = active && s.value;
		}

		// If we havent already returned we are dealing with the last input
		auto& last = inputStates.back();
		last.value = is.value;
		active = active && last.value;
	}

	bool InputBindMapping::isActive() const {
		return active;
	}

	BindId InputBindMapping::getBindId() const {
		return bid;
	}
};
