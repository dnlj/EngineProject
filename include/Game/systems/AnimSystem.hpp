#pragma once

// Engine
#include <Engine/Gfx/ModelLoader.hpp>

// Game
#include <Game/System.hpp>


namespace Game {
	class AnimSystem : public System {
		private:
			Engine::Gfx::Mesh test;
			Engine::ShaderRef shader;
			GLuint ubo;

			Engine::Gfx::Animation animation;
			Engine::Gfx::Armature arm;

			std::vector<glm::mat4> bonesFinal;

		public:
			AnimSystem(SystemArg arg);
			~AnimSystem();
			void updateAnim();
			void render(const RenderLayer layer);
	};
}
