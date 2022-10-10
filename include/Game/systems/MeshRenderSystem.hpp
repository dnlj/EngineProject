#pragma once

// Engine
#include <Engine/Gfx/resources.hpp>
#include <Engine/Gfx/TextureHandle.hpp>
#include <Engine/Gfx/ActiveTextureCache.hpp>

// Game
#include <Game/System.hpp>


namespace Game {
	class MeshRenderSystem : public System {
		private:
			Engine::Gfx::BufferRef matParamsBuffer;

			constexpr static uint32 maxActiveTextures = 16; // 16 = OpenGL fragment texture limit
			Engine::Gfx::ActiveTextureCache<maxActiveTextures> texCache;
			Engine::Gfx::TextureHandleGeneric texActive[maxActiveTextures];
			int32 texIndices[maxActiveTextures];

			Engine::Gfx::Texture2DRef errTexture;

		public:
			MeshRenderSystem(SystemArg arg);
			void render(RenderLayer layer);
	};
}
