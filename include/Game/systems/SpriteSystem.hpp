#pragma once

// STD
#include <vector>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Engine
#include <Engine/EngineInstance.hpp>

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
				GLuint texture = 0;
				GLsizei count = 0;
				GLuint base = 0;
			};

		public:
			struct Sprite {
				GLuint texture;
				glm::mat4 trans;
			};

		public:
			SpriteSystem(SystemArg arg);
			~SpriteSystem();

			void run(float32 dt);
			void render(const RenderLayer layer);
			void addSprite(Sprite sprite);

		private:
			constexpr static std::size_t MAX_SPRITES = 1024;
			std::vector<InstanceData> instanceData;
			std::vector<SpriteGroup> spriteGroups;
			std::vector<Sprite> sprites;

			Engine::Shader shader;
			GLuint vao = 0;
			GLuint vbo = 0;
			GLuint ivbo = 0;
			GLuint ebo = 0;
	};
}
