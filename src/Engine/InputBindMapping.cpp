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
		// TODO: Currently order of binds matter. I think we probably only want the last key's order to matter. (Ex [not currently the case]: CTRL + ALT + Q == ALT + CTRL + Q)
		for (int i = 0; i < inputStates.size(); ++i) {
			auto& s = inputStates[i];

			if (s.input == is.input){
				s.state = is.state;
			} else if (!s.input) {
				break;
			}

			if (!s.state) {
				for (; i < inputStates.size(); ++i) {
					inputStates[i].state = false;
				}

				active = false;
				return;
			}
		}
		active = true;
	}

	bool InputBindMapping::isActive() const {
		return active;
	}

	BindId InputBindMapping::getBindId() const {
		return bid;
	}
};
