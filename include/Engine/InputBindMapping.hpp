#pragma once

// STD
#include <array>

// Engine
#include <Engine/FlatHashMap.hpp>
#include <Engine/Input.hpp>


// TODO: Doc
// TODO: split
namespace Engine {
	using BindId = int;

	class InputBindMapping {
		public:
			InputBindMapping(InputSequence inputs, BindId bid)
				: bid{bid} {

				for (int i = 0; i < inputs.size(); ++i) {
					inputStates[i].input = std::move(inputs[i]);
				}
			}

			void processInput(const InputState& is) {
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

			bool isActive() const {
				return active;
			}

			BindId getBindId() const {
				return bid;
			}

		private:
			bool active = false;
			const BindId bid;
			std::array<InputState, std::tuple_size<InputSequence>::value> inputStates;
	};
};
