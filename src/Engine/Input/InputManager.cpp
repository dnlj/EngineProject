// Engine
#include <Engine/Input/InputManager.hpp>


namespace Engine::Input {
	bool InputManager::Layer::processInput(const InputManager& manager, const InputState& state) {

		if (state.id.type != InputType::MOUSE_AXIS) {
			ENGINE_LOG("*** State: ", (int64)state.id.type, " ", state.id.code, " ", state.value.i32);
		}


		if (state.value.any()) { // Activate binds
			auto found = lookup.find(state.id);
			if (found == lookup.end()) { return false; }

			const auto last = found->second.end();
			for (auto bind = found->second.begin(); bind != last; ++bind) {
				bool update = true;
				const auto lim = bind->inputs.size() - 1;
				for (int i = 0; i < lim; ++i) {
					update = update && manager.getTrackedValue(bind->inputs[i]).any();
				}

				if (update) {
					if (!bind->value.any()) {
						//
						// TODO: axis inputs shouldnt get added to active list.
						//
						active.push_back(bind);
						ENGINE_LOG("Add ", active.size());
					}

					bind->listener(state.value, bind->value);
					bind->value = state.value;
					return true;
				}
			}
		} else { // Deactivate any relevant binds
			//
			// TODO: axis inputs shouldnt be checked since they shouldnt be in active list
			//
			auto lastActive = active.size();
			for (int b = 0; b < lastActive; ++b) {
				auto& bind = active[b];
				const auto lim = bind->inputs.size();
				for (int i = 0; i < lim; ++i) {
					auto x = bind->inputs[i];
					auto y = state.id;
					ENGINE_LOG(
						"\n x = ", x,
						"\n y = ", y
					);
					if (bind->inputs[i] == state.id) {
						bind->listener({}, bind->value);
						bind->value = {};
						ENGINE_LOG("Remove ", active.size());

						for (auto j : bind->inputs) {
							ENGINE_LOG("    ", (int)j.type, " ", (int)j.code);
						}
						bind = active[--lastActive];
						active.pop_back();
						--b;
						break;
					}
				}
			}
		}

		// TODO: need a template param for fallthrough callback to translate chars

		return false;
	}
}

namespace Engine::Input {
	void InputManager::processInput(const InputState& is) {
		const auto found = bindLookup.find(is.id);
		if (found == bindLookup.end()) { return; }
		
		for (const auto bid : found->second) {
			auto& bind = binds[static_cast<std::underlying_type_t<decltype(bid)>>(bid)];
			
			const auto pre = bind.getState();
			bind.processInput(is);
			const auto post = bind.getState();
			
			if (pre != post) {
				bind.notify(post, pre);
			}
		}
	}
}
