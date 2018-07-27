#pragma once

// Game
#include <Game/Common.hpp>


namespace Game {
	class CharacterSpellSystem : public SystemBase {
		public:
			CharacterSpellSystem(World& world);
			virtual void run(float dt) override;

		private:
			Engine::ECS::EntityFilter& filter;
	};
}
