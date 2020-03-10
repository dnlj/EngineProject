#pragma once

// STD
#include <vector>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Engine
#include <Engine/EngineInstance.hpp>
#include <Engine/ECS/EntityFilter.hpp>

// Game
#include <Game/System.hpp>


namespace Game {
	class SpriteSystem : public SystemBase {
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
				glm::vec3 position;
			};

		public:
			SpriteSystem(World& world);
			~SpriteSystem();

			void setup(Engine::EngineInstance& engine);
			void run(float dt);

			void addSprite(Sprite sprite);

		private:
			Engine::ECS::EntityFilter& filter;

			const Engine::Camera* camera;

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
