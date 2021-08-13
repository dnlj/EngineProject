// Engine
#include <Engine/Input/InputManager.hpp>


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
