#pragma once

// STD
#include <vector>
#include <string>

// Engine
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
						printf("Press: %s\n", binds[map.getBindId()].name.c_str());
					} else if (preActive && !postActive) {
						binds[map.getBindId()].release();
						printf("Release: %s\n", binds[map.getBindId()].name.c_str());
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

				return -1;
			}

			void addInputBindMapping(InputSequence inputs, std::string bind) {
				const auto bid = getBindId(bind);

				#ifdef DEBUG
					if (!inputs[0]) {
						// TODO: Error: must have at least one key
					}

					if (bid < 0) {
						// TODO: Error: invalid bind id
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
