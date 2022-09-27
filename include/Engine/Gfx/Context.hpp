#pragma once

// Engine
#include <Engine/Gfx/DrawCommand.hpp>
#include <Engine/Gfx/resources.hpp>
#include <Engine/Gfx/TextureHandle.hpp>
#include <Engine/Gfx/ActiveTextureCache.hpp>



namespace Engine::Gfx {
	class Context {
		private:
			std::vector<DrawCommand> cmds;
			DrawCommand active = {};
			BufferRef matParamsBuffer;
			uint32 matParamsBufferSize = 0;

			constexpr static uint32 maxActiveTextures = 16; // 16 = OpenGL fragment texture limit
			ActiveTextureCache<maxActiveTextures> texCache;
			TextureHandleGeneric texActive[maxActiveTextures];
			int32 texIndices[maxActiveTextures];

			Texture2DRef errTexture;

		public:
			Context(BufferManager& bufferManager, TextureLoader& textureLoader);
			void push(const DrawCommand& cmd) { cmds.push_back(cmd); }
			void render();
	};
}
