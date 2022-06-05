#pragma once

// Engine
#include <Engine/Gfx/ModelLoader.hpp>
#include <Engine/Gfx/VertexLayoutLoader.hpp>
#include <Engine/Gfx/BufferManager.hpp>
#include <Engine/Gfx/Buffer.hpp>
#include <Engine/Gfx/ShaderLoader.hpp>
#include <Engine/Gfx/MeshManager.hpp>

// Game
#include <Game/System.hpp>


namespace Game {
	class ModelData {
		public:
			bool skinned = false;
			Engine::Gfx::Armature arm;
			std::vector<glm::mat4> bones;
			std::vector<Engine::Gfx::MeshInst> instances;
			std::vector<Engine::Gfx::MeshRef> meshes;
	};

	class AnimSystem : public System {
		private:
			Engine::Gfx::VertexAttributeLayoutRef layout;
			Engine::Gfx::ShaderRef shaderSkinned;
			Engine::Gfx::ShaderRef shaderStatic;
			Engine::Gfx::BufferRef ubo;

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
