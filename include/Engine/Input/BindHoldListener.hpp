#pragma once

// Engine
#include <Engine/Input/BindListener.hpp>


namespace Engine::Input {
	class BindHoldListener : public BindListener {
		private:
			friend class ButtonBind;
			virtual void onBindHold() = 0;
	};
}
