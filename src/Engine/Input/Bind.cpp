// Engine
#include <Engine/Input/Bind.hpp>


namespace Engine::Input {
	Bind::Bind(InputSequence inputs, BindListener listener)
		: listener{listener} {
		inputStates.resize(inputs.size());

		for (int i = 0; i < inputs.size(); ++i) {
			inputStates[i].id = std::move(inputs[i]);
		}
	}

	void Bind::processInput(const InputState& is) {
		bool update = true;

		for (int i = 0; i < inputStates.size() - 1; ++i) {
			auto& s = inputStates[i];
			if (s.id == is.id) {
				s.value = is.value;
				return; // There should only be one state for each input. No need to check the rest.
			}

			update = update && s.value;
		}

		// If we havent already returned we are dealing with the last input
		auto& last = inputStates.back();
		last.value = is.value;
		if (update) {
			state.value = last.value;
		}
	}

	Value Bind::getState() const {
		return state;
	}

	void Bind::notify(Value curr, Value prev) const {
		listener(curr, prev);
	}
};
