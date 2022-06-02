#pragma once

// Engine
#include <Engine/Gfx/ModelLoader.hpp>
#include <Engine/Gfx/VertexLayoutLoader.hpp>
#include <Engine/Gfx/BufferManager.hpp>
#include <Engine/Gfx/Buffer.hpp>

// Game
#include <Game/System.hpp>


namespace Game {
	class ModelData {
		public:
			bool skinned = false;
			Engine::Gfx::Armature arm;
			std::vector<glm::mat4> bones;
			std::vector<Engine::Gfx::MeshInst> instances;
			std::vector<Engine::Gfx::MeshDesc> meshes;
	};

	class AnimSystem : public System {
		private:
			Engine::Gfx::VertexAttributeLayoutRef layout;
			Engine::ShaderRef shaderSkinned;
			Engine::ShaderRef shaderStatic;
			Engine::Gfx::BufferRef ubo;
			Engine::Gfx::BufferRef vbo;
			Engine::Gfx::BufferRef ebo;

			GLuint cmdbuff = 0; // TODO: rm - just for testing

			Engine::Gfx::Animation animation;
			ModelData model;

		public:
			AnimSystem(SystemArg arg);
			~AnimSystem();
			void updateAnim();
			void render(const RenderLayer layer);
	};
}
