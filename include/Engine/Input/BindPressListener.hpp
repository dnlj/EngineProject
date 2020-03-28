#pragma once

// Engine
#include <Engine/Input/BindListener.hpp>


namespace Engine::Input {
	class BindPressListener : public BindListener {
		private:
			friend class ButtonBind;
			virtual void onBindPress() = 0;
	};
}
