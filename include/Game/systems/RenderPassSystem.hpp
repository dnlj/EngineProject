#pragma once

// Game
#include <Game/System.hpp>


namespace Game {
	class RenderPassSystem : public System {
		public:
			using System::System;
			void run(float32 dt);
	};
}
