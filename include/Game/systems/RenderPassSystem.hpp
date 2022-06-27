#pragma once

// Game
#include <Game/System.hpp>


namespace Game {
	class RenderPassSystem : public System {
		public:
			using System::System;
			void update(float32 dt);
	};
}
