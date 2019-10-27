#pragma once

// STD
#include <vector>
#include <string>

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/Input/Bind.hpp>
#include <Engine/Input/InputId.hpp>
#include <Engine/Input/InputBindMapping.hpp>
#include <Engine/Input/InputState.hpp>
#include <Engine/Input/InputSequence.hpp>


// TODO: Doc
// TODO: Move all this input stuff into a namespace
// TODO: split
namespace Engine::Input {
	class InputManager {
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
						binds[map.getBindId()].press();
					} else if (preActive && !postActive) {
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

			// TODO: remove? Represent mouse/axis as a bind?
			/**
			 * Gets the current position of the mouse.
			 * Origin is top left
			 * @return The x and y position of the mouse.
			 */
			glm::vec2 getMousePosition() const {
				return mousePosition;
			}

			// TODO: remove? Represent mouse/axis as a bind?
			/**
			 * The callback for updating the mouse position.
			 * See GLFW documentation for more information.
			 * @param[in] x The x position of the mouse.
			 * @param[in] y The y position of the mouse.
			 */
			void mouseCallback(double x, double y) {
				mousePosition.x = static_cast<float>(x);
				mousePosition.y = static_cast<float>(y);
			}

		private:
			FlatHashMap<InputId, std::vector<uint16_t>, Hash<InputId>> inputToMapping;
			std::vector<InputBindMapping> inputBindMappings;
			std::vector<Bind> binds;
			glm::vec2 mousePosition; // TODO: remove? Represent mouse/axis as a bind?
	};
}
