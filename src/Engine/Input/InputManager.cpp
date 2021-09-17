// Engine
#include <Engine/Input/InputManager.hpp>


namespace Engine::Input {
	bool InputManager::Layer::processInput(const InputManager& manager, const InputState& state) {
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
					if (!bind->value.any() && !isAxisInput(state.id.type)) {
						active.push_back(bind);
					}

					bind->listener(state.value, bind->value);
					bind->value = state.value;
					return true;
				}
			}
		} else if (!isAxisInput(state.id.type)) { // Deactivate any relevant binds
			auto lastActive = active.size();
			for (int b = 0; b < lastActive; ++b) {
				auto& bind = active[b];
				const auto lim = bind->inputs.size();
				for (int i = 0; i < lim; ++i) {
					if (bind->inputs[i] == state.id) {
						bind->listener({}, bind->value);
						bind->value = {};
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
