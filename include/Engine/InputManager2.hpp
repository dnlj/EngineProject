#pragma once

// STD
#include <vector>
#include <string>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/Bind.hpp>
#include <Engine/Input.hpp>
#include <Engine/InputBindMapping.hpp>
#include <Engine/InputState.hpp>
#include <Engine/InputSequence.hpp>


namespace Engine {
	class InputManager2 {
		public:
			void processInput(const InputState& is) {
				const auto found = inputToMapping.find(is.input);
				if (found == inputToMapping.end()) { return; }

				for (const auto mapi : found->second) {
					auto& map = inputBindMappings[mapi];

					const bool preActive = map.isActive();
					map.processInput(is);
					const bool postActive = map.isActive();

					if (!preActive && postActive) {
						binds[map.getBindId()].press();
					} else if (preActive && !postActive) {
						binds[map.getBindId()].release();
					}
				}
			}

			void createBind(std::string name) {
				binds.emplace_back(std::move(name));
			}

			BindId getBindId(const std::string& name) const {
				for (int i = 0; i < binds.size(); ++i) {
					if (binds[i].name == name) {
						return i;
					}
				}

				#ifdef DEBUG
					ENGINE_ERROR("Invalid bind name: " << name);
				#endif
			}

			Bind& getBind(const std::string& name) {
				return binds[getBindId(name)];
			}

			void addInputBindMapping(InputSequence inputs, std::string bind) {
				const auto bid = getBindId(bind);

				#ifdef DEBUG
					if (!inputs[0]) {
						ENGINE_ERROR("InputSequence must have at least one input.");
					}
				#endif

				inputBindMappings.emplace_back(std::move(inputs), bid);

				for (const auto& input : inputs) {
					if (input) {
						inputToMapping[input].push_back(
							static_cast<uint16_t>(inputBindMappings.size() - 1)
						);
					}
				}
			}

		private:
			FlatHashMap<Input, std::vector<uint16_t>, Hash<Input>> inputToMapping;
			std::vector<InputBindMapping> inputBindMappings;
			std::vector<Bind> binds;
	};
}
