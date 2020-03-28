// Engine
#include <Engine/Input/ButtonBind.hpp>

namespace Engine::Input {
	ButtonBind::ButtonBind(std::string name) : name{std::move(name)} {
	}

	void ButtonBind::press() {
		if (active == 0) {
			for (auto l : pressListeners) {
				l->onBindPress();
			}
		}

		++active;
	};

	void ButtonBind::hold() const {
		for (auto l : holdListeners) {
			l->onBindHold();
		}
	};

	void ButtonBind::release() {
		--active;

		if (active == 0) {
			for (auto l : releaseListeners) {
				l->onBindRelease();
			}
		}
	};

	bool ButtonBind::isActive() const {
		return active;
	}

	void ButtonBind::addPressListener(BindPressListener* listener) {
		pressListeners.push_back(listener);
	}

	void ButtonBind::addHoldListener(BindHoldListener* listener) {
		holdListeners.push_back(listener);
	}
	
	void ButtonBind::addReleaseListener(BindReleaseListener* listener) {
		releaseListeners.push_back(listener);
	}
}
