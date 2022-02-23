// Engine
#include <Engine/Input/BindManager.hpp>


namespace Engine::Input {
	bool BindManager::Layer::processInput(const BindManager& manager, const InputEvent& event) {
		if (event.state.value.any()) { // Activate binds
			auto found = lookup.find(event.state.id);
			if (found == lookup.end()) { return false; }

			const auto last = found->second.end();
			for (auto bind = found->second.begin(); bind != last; ++bind) {
				bool update = true;

				for (const auto& input : bind->inputs) {
					update = update && manager.getTrackedValue(input).any();
				}

				if (update) {
					if (!bind->value.any() && !isAxisInput(event.state.id.type)) {
						active.push_back(bind);
					}

					if (bind->repeat || event.state.value != bind->value) {
						const auto consume = bind->listener(event.state.value, bind->value, event.time);
						bind->value = event.state.value;
						return consume;
					}
				}
			}
		} else if (!isAxisInput(event.state.id.type)) { // Deactivate any relevant binds
			auto lastActive = active.size();
			for (int b = 0; b < lastActive; ++b) {
				auto& bind = active[b];

				for (const auto& input : bind->inputs) {
					if (input == event.state.id) {
						bind->listener({}, bind->value, event.time);
						bind->value = {};
						bind = active[--lastActive];
						active.pop_back();
						--b;
						break;
					}
				}
			}
		}

		return false;
	}
}
