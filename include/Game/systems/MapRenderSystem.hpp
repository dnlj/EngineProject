#pragma once

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Game
#include <Game/Common.hpp>
#include <Game/MapChunk.hpp>


namespace Game {
	class MapRenderSystem : public System {
		public:
			MapRenderSystem(SystemArg arg);
			~MapRenderSystem();

			void update(float dt);
			void render(const RenderLayer layer);
	};
}
