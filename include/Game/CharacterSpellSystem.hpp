#pragma once

// Engine
#include <Engine/EngineInstance.hpp>

// Game
#include <Game/Common.hpp>
#include <Game/PhysicsUserData.hpp>


namespace Game {
	class CharacterSpellSystem : public SystemBase {
		public:
			CharacterSpellSystem(World& world);
			void setup(Engine::EngineInstance& engine);
			virtual void run(float dt) override;

		private:
			Engine::ECS::EntityFilter& filter;
			Engine::Camera* camera;
			std::vector<Engine::ECS::Entity> missles;
			std::vector<PhysicsUserData> userData;
			size_t currentMissle = 0;
	};
}
