#pragma once

// Engine
#include <Engine/Gfx/ModelLoader.hpp>

// Game
#include <Game/System.hpp>


namespace Game {
	class AnimSystem : public System {
		private:
			Engine::Gfx::Mesh test;
			Engine::ShaderRef shaderSkinned;
			Engine::ShaderRef shaderStatic;
			GLuint ubo;

			GLuint cmdbuff = 0; // TODO: rm - just for testing

			Engine::Gfx::Animation animation;
			Engine::Gfx::Armature arm;
			std::vector<Engine::Gfx::MeshInst> instances;

			std::vector<glm::mat4> bonesFinal;
			std::vector<Engine::Gfx::MeshRange> meshes;

		public:
			AnimSystem(SystemArg arg);
			~AnimSystem();
			void updateAnim();
			void render(const RenderLayer layer);
	};
}
