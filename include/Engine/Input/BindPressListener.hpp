#pragma once

// Engine
#include <Engine/Input/BindListener.hpp>


namespace Engine::Input {
	class BindPressListener : public BindListener {
		private:
			friend class Bind;
			virtual void onBindPress() = 0;
	};
}
