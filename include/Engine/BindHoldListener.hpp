#pragma once

// Engine
#include <Engine/BindListener.hpp>


namespace Engine {
	class BindHoldListener : public BindListener {
		private:
			friend class Bind;
			virtual void onBindHold() = 0;
	};
}
