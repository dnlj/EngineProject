#pragma once

// STD
#include <vector>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Engine
#include <Engine/Gfx/ShaderLoader.hpp>

// Game
#include <Game/System.hpp>


namespace Game {
	class SpriteSystem : public System {
		private:
			struct Vertex {
				glm::vec2 position;
				glm::vec2 texCoord;
			};

			struct InstanceData {
				glm::mat4 mvp;
			};

			struct SpriteGroup {
				RenderLayer layer;
				GLuint texture = 0;
				GLsizei count = 0;
				GLuint base = 0;
			};

			struct Sprite {
				RenderLayer layer;
				GLuint texture;
				glm::mat4 trans;
			};

		private:
			constexpr static GLuint dataBindingIndex = 0;
			constexpr static GLuint instBindingIndex = 1;

			std::vector<InstanceData> instanceData;
			std::vector<SpriteGroup> spriteGroups;
			std::vector<Sprite> sprites;

			int nextGroup = 0;
			RenderLayer nextLayer = RenderLayer::_count;
			Engine::Gfx::ShaderRef shader;
			GLuint vao = 0;
			GLuint vbo = 0;
			GLuint ivbo = 0;
			GLuint ebo = 0;

		private:
			void resizeInstanceData();

		public:
			SpriteSystem(SystemArg arg);
			~SpriteSystem();

			void run(float32 dt);
			void render(const RenderLayer layer);
			void addSprite(Sprite sprite);
			ENGINE_INLINE auto totalSprites() const noexcept { return sprites.size(); }
			ENGINE_INLINE auto totalSpriteGroups() const noexcept { return spriteGroups.size(); }
			ENGINE_INLINE const auto& getSpriteGroups() const noexcept { return spriteGroups; }
	};
}
