#pragma once

// Engine
#include <Engine/Input/ButtonBind.hpp>

namespace Engine::Input {
	template<class Listener, class>
	void ButtonBind::addListener(Listener* listener) {
		if constexpr (std::is_base_of_v<BindPressListener, Listener>) {
			addPressListener(listener);
		}

		if constexpr (std::is_base_of_v<BindHoldListener, Listener>) {
			addHoldListener(listener);
		}

		if constexpr (std::is_base_of_v<BindReleaseListener, Listener>) {
			addReleaseListener(listener);
		}
	};
}
