#pragma once

// STD
#include <vector>

// Engine
#include <Engine/Camera.hpp>

// Game
#include <Game/Common.hpp>


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
			SpriteSystem(World& world);
			~SpriteSystem();

			void setup(const Engine::Camera& camera);
			void run(float dt);

		private:
			Engine::ECS::EntityFilter& filter;

			const Engine::Camera* camera;

			constexpr static std::size_t MAX_SPRITES = 1024;
			std::vector<InstanceData> instanceData;

			std::vector<SpriteGroup> spriteGroups;

			GLuint shader = 0;
			GLuint vao = 0;
			GLuint vbo = 0;
			GLuint ivbo = 0;
			GLuint ebo = 0;
	};
}
