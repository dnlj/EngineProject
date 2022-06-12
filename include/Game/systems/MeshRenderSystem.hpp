#pragma once

// Game
#include <Game/System.hpp>


namespace Game {
	class MeshRenderSystem : public System {
		public:
			MeshRenderSystem(SystemArg arg);
			void render(RenderLayer layer);
	};
}
