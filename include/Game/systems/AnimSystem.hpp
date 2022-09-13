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
			Engine::ECS::Entity ents[4];

			Engine::Gfx::Animation animation;
			bool skinned = false;

			std::vector<byte> bonesBuffTemp;
			Engine::Gfx::BufferRef bonesBuff;
			uint64 bonesBuffSize = 0;

		public:
			AnimSystem(SystemArg arg);
			~AnimSystem();
			void updateAnim();
			void render(const RenderLayer layer);
	};
}
