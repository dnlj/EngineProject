// Engine
#include <Engine/InputBindMapping.hpp>

namespace Engine {
	InputBindMapping::InputBindMapping(InputSequence inputs, BindId bid)
		: bid{bid} {

		for (int i = 0; i < inputs.size(); ++i) {
			inputStates[i].input = std::move(inputs[i]);
		}
	}

	void InputBindMapping::processInput(const InputState& is) {
		active = true;

		for (auto& s : inputStates) {
			if (s.input == is.input) {
				s.state = is.state;
			} else if (!s.input) {
				break;
			}

			active = active && s.state;
		}
	};

	bool InputBindMapping::isActive() const {
		return active;
	}

	BindId InputBindMapping::getBindId() const {
		return bid;
	}
};
