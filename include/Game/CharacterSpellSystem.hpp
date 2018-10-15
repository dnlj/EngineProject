#pragma once

// Engine
#include <Engine/EngineInstance.hpp>

// Game
#include <Game/Common.hpp>
#include <Game/PhysicsListener.hpp>


namespace Game {
	class CharacterSpellSystem : public SystemBase, public PhysicsListener {
		public:
			CharacterSpellSystem(World& world);
			void setup(Engine::EngineInstance& engine);
			virtual void run(float dt) override;

		private:
			virtual void beginContact(const PhysicsUserData& dataA, const PhysicsUserData& dataB) override;

			void fireMissile(Engine::ECS::Entity ent, Engine::InputManager& inputManager);
			void detonateMissle(Engine::ECS::Entity ent);

			Engine::ECS::EntityFilter& filter;
			Engine::Camera* camera;
			std::vector<Engine::ECS::Entity> missles;
			std::vector<Engine::ECS::Entity> toDestroy;
			size_t currentMissle = 0;
	};
}
