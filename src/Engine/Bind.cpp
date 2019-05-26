// Engine
#include <Engine/Bind.hpp>

namespace Engine {
	Bind::Bind(std::string name) : name{std::move(name)} {
	}

	void Bind::press() {
		if (active == 0) {
			for (auto l : pressListeners) {
				l->onBindPress();
			}
		}

		++active;
	};

	void Bind::release() {
		--active;

		if (active == 0) {
			for (auto l : releaseListeners) {
				l->onBindRelease();
			}
		}
	};

	void Bind::addPressListener(BindPressListener* listener) {
		pressListeners.push_back(listener);
	}

	void Bind::addReleaseListener(BindReleaseListener* listener) {
		releaseListeners.push_back(listener);
	}
}
