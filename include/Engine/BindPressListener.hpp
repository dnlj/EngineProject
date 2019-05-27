#pragma once

// Engine
#include <Engine/BindListener.hpp>


namespace Engine {
	class BindPressListener : public BindListener {
		private:
			friend class Bind;
			virtual void onBindPress() = 0;
	};
}
