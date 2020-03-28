#pragma once

// Engine
#include <Engine/Input/BindListener.hpp>


namespace Engine::Input {
	class BindReleaseListener : public BindListener {
		private:
			friend class ButtonBind;
			virtual void onBindRelease() = 0;
	};
}
