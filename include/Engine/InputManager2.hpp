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


// TODO: Doc
// TODO: Move all this input stuff into a namespace
// TODO: split
namespace Engine {
	class InputManager2 {
		public:
			void update() {
				for (const auto& bind : binds) {
					if (bind.isActive()) {
						bind.hold();
					}
				}
			}

			void processInput(const InputState& is) {
				const auto found = inputToMapping.find(is.input);
				if (found == inputToMapping.end()) { return; }

				for (const auto mapi : found->second) {
					auto& map = inputBindMappings[mapi];

					const bool preActive = map.isActive();
					map.processInput(is);
					const bool postActive = map.isActive();

					if (!preActive && postActive) {
						puts("Bind Press");
						binds[map.getBindId()].press();
					} else if (preActive && !postActive) {
						puts("Bind Release");
						binds[map.getBindId()].release();
					}
				}
			}

			BindId createBind(std::string name) {
				binds.emplace_back(std::move(name));
				return static_cast<BindId>(binds.size()) - 1;
			}

			BindId getBindId(const std::string& name) const {
				for (int i = 0; i < binds.size(); ++i) {
					if (binds[i].name == name) {
						return i;
					}
				}

				return -1;
			}

			Bind& getBind(const std::string& name) {
				return binds[getBindId(name)];
			}

			Bind& getBind(const BindId bid) {
				return binds[bid];
			}

			void addInputBindMapping(InputSequence inputs, const std::string& name) {
				#ifdef DEBUG
					if (inputs.size() < 1) {
						ENGINE_ERROR("InputSequence must have at least one input.");
					}
				#endif

				auto bid = getBindId(name);
				if (bid < 0) {
					bid = createBind(name);
				}

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
