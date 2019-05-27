#pragma once

// Engine
#include <Engine/BindListener.hpp>


namespace Engine {
	class BindReleaseListener : public BindListener {
		private:
			friend class Bind;
			virtual void onBindRelease() = 0;
	};
}
