#pragma once

// STD
#include <vector>
#include <string>
#include <string_view>
#include <type_traits>
#include <tuple>
#include <forward_list>

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/Input/InputId.hpp>
#include <Engine/Input/BindId.hpp>
#include <Engine/Input/InputSequence.hpp>
#include <Engine/Input/BindListener.hpp>


namespace Engine::Input {
	enum class InputLayer {
		GUI,
		Game,
	};

	class Bind2 {
		public:
			InputSequence inputs;
			BindListener listener;
			Value value;
	};

	class InputManager {
		private:
			class Layer {
				private:
					using BindList = std::forward_list<Bind2>;

					/** Mapping from activating inputs to binds. */
					FlatHashMap<InputId, BindList> lookup;

					/** List of active binds. Needed for bind deactivation for non-activating inputs. */
					std::vector<BindList::iterator> active;

					/** Is this layer enabled? */
					bool enabled;

				public:
					void addBind(const InputSequence& inputs, BindListener listener) {
						if (inputs.empty()) [[unlikely]] {
							ENGINE_WARN("Attempting to add invalid bind. Ignoring.");
							return;
						}

						lookup[inputs.back()].push_front({
							.inputs = inputs,
							.listener = listener,
							.value = {},
						});
					}

					bool processInput(const InputManager& manager, const InputState& state);

					ENGINE_INLINE bool getEnabled() const noexcept { return enabled; }
					ENGINE_INLINE void setEnabled(bool val) noexcept { enabled = val; }
			};

		public:
			Layer layers[1];
			FlatHashMap<InputId, Value> inputStates;
			
		public:
			void enableLayer(int layer);
			void disableLayer(int layer);

			ENGINE_INLINE Value getTrackedValue(const InputId id) const noexcept {
				auto found = inputStates.find(id);
				return found != inputStates.end() ? found->second : Value{};
			}

			void addBind(int layer, const InputSequence& inputs, BindListener listener) {
				ENGINE_DEBUG_ASSERT(layer < std::size(layers));
				auto& lay = layers[layer];
				lay.addBind(inputs, std::move(listener));

				// Make sure all values are tracked
				for (auto i : inputs) { inputStates[i]; }
			}

			void processInput(const InputState& is) {
				auto found = inputStates.find(is.id);
				if (found == inputStates.end()) { return; }
				found->second = is.value;

				for (auto& layer : layers) {
					if (!layer.getEnabled()) { continue; }
					if (layer.processInput(*this, is)) { break; }
				}
			}
	};
}
