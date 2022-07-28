#pragma once

// Engine
#include <Engine/Gfx/ModelLoader.hpp> // TODO: rm - needed for `Gfx::Animation` which should be moved to own file.
#include <Engine/Gfx/resources.hpp>

// Game
#include <Game/System.hpp>


namespace Game {
	class ModelData {
		public:
			struct Inst {
				Engine::Gfx::NodeId nodeId;
				Engine::Gfx::MeshRef mesh;
				Engine::Gfx::MaterialInstanceRef material;
			};

		public:
			bool skinned = false;
			//Engine::Gfx::Armature arm;
			//std::vector<glm::mat4> bones;
	};

	class AnimSystem : public System {
		private:
			Engine::ECS::Entity ent;
			Engine::Gfx::BufferRef ubo;

			GLuint cmdbuff = 0; // TODO: rm - just for testing

			Engine::Gfx::Animation animation;
			ModelData model;

			Engine::Gfx::MaterialInstanceRef mats[3];

		public:
			AnimSystem(SystemArg arg);
			~AnimSystem();
			void updateAnim();
			void render(const RenderLayer layer);
	};
}
