#pragma once

// Engine
#include <Engine/Input/BindListener.hpp>


namespace Engine::Input {
	class BindReleaseListener : public BindListener {
		private:
			friend class Bind;
			virtual void onBindRelease() = 0;
	};
}
