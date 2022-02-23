#pragma once

// STD
#include <vector>
#include <forward_list>

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/Input/InputEvent.hpp>
#include <Engine/Input/InputSequence.hpp>


namespace Engine::Input {
	enum class BindId : uint16;

	using BindListener = std::function<bool(Value curr, Value prev, Clock::TimePoint time)>;

	class Bind {
		public:
			InputSequence inputs;
			BindListener listener;
			Value value;
			bool repeat;
	};
	
	class BindManager {
		private:
			class Layer {
				private:
					using BindList = std::forward_list<Bind>;

					/** Mapping from activating inputs to binds. */
					FlatHashMap<InputId, BindList> lookup;

					/** List of active binds. Needed for bind deactivation for non-activating inputs. */
					std::vector<BindList::iterator> active;

					/** Is this layer enabled? */
					bool enabled = true;

				public:
					void addBind(const InputSequence& inputs, bool repeat, BindListener listener) {
						if (inputs.empty()) [[unlikely]] {
							ENGINE_WARN("Attempting to add invalid bind. Ignoring.");
							return;
						}

						lookup[inputs.back()].push_front({
							.inputs = inputs,
							.listener = listener,
							.value = {},
							.repeat = repeat,
						});
					}

					bool processInput(const BindManager& manager, const InputEvent& event);

					ENGINE_INLINE bool getEnabled() const noexcept { return enabled; }
					ENGINE_INLINE void setEnabled(bool val) noexcept { enabled = val; }
			};

		public:
			std::vector<Layer> layers;
			FlatHashMap<InputId, Value> inputStates;
			
		public:
			template<class L>
			ENGINE_INLINE void setLayerEnabled(L layer, bool val) {
				layers[static_cast<int64>(layer)].setEnabled(val);
			}

			template<class L>
			ENGINE_INLINE bool getLayerEnabled(L layer) const noexcept {
				return layers[static_cast<int64>(layer)].getEnabled();
			}

			ENGINE_INLINE Value getTrackedValue(const InputId id) const noexcept {
				auto found = inputStates.find(id);
				return found != inputStates.end() ? found->second : Value{};
			}

			template<class L>
			void addBind(L layer, bool repeat, const InputSequence& inputs, BindListener listener) {
				using T = std::conditional_t<std::is_enum_v<L>, std::underlying_type_t<L>, L>;
				const auto layert = static_cast<T>(layer);
				if (layert >= layers.size()) {
					ENGINE_ASSERT_WARN(layert < layers.size() + 10, "Inserting a large number of input layers. Was this intended?");
					layers.resize(layert + 1);
					ENGINE_LOG("Add layer: ", layert);
				}

				auto& lay = layers[layert];
				lay.addBind(inputs, repeat, std::move(listener));

				// Make sure all values are tracked
				for (auto i : inputs) { inputStates[i]; }
			}

			void processInput(const InputEvent& ie) {
				auto found = inputStates.find(ie.state.id);
				if (found == inputStates.end()) { return; }
				found->second = ie.state.value;

				for (auto& layer : layers) {
					if (!layer.getEnabled()) { continue; }
					if (layer.processInput(*this, ie)) { break; }
				}
			}
	};
}
