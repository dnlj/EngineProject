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
			std::vector<byte> bonesBuffTemp;
			Engine::Gfx::BufferRef bonesBuff;
			uint64 bonesBuffSize = 0;

			std::vector<glm::mat4> mvpBuffTemp;
			Engine::Gfx::BufferRef mvpBuff;
			uint64 mvpBuffSize = 0;

			std::vector<uint32> idBuffTemp;
			Engine::Gfx::BufferRef idBuff;
			uint64 idBuffSize = 0;

		public:
			AnimSystem(SystemArg arg);
			~AnimSystem();
			void render(const RenderLayer layer);
	};
}
