#pragma once

// Engine
#include <Engine/Gfx/Animation.hpp>
#include <Engine/Gfx/Armature.hpp>
#include <Engine/Gfx/resources.hpp>

// Game
#include <Game/System.hpp>


namespace Game {
	class AnimSystem : public System {
		private:
			Engine::ECS::Entity ent;
			Engine::Gfx::BufferRef ubo;

			GLuint cmdbuff = 0; // TODO: rm - just for testing

			Engine::Gfx::Animation animation;
			bool skinned = false;

			Engine::Gfx::MaterialInstanceRef mats[3];

		public:
			AnimSystem(SystemArg arg);
			~AnimSystem();
			void updateAnim();
			void render(const RenderLayer layer);
	};
}
