#pragma once

// Engine
#include <Engine/EngineInstance.hpp>

// Game
#include <Game/Common.hpp>
#include <Game/PhysicsListener.hpp>


namespace Game {
	class CharacterSpellSystem : public SystemBase {
		public:
			CharacterSpellSystem(World& world);
			void setup(Engine::EngineInstance& engine);
			virtual void run(float dt) override;

		private:
			class CollisionListener : public PhysicsListener {
				public:
					CollisionListener(CharacterSpellSystem& spellSys);
					virtual void beginContact(const PhysicsUserData& dataA, const PhysicsUserData& dataB) override;

				private:
					CharacterSpellSystem& spellSys;
			};

			void fireMissile(Engine::ECS::Entity ent, Engine::InputManager& inputManager);

			CollisionListener collisionListener;
			Engine::ECS::EntityFilter& filter;
			Engine::Camera* camera;
			std::vector<Engine::ECS::Entity> missles;
			size_t currentMissle = 0;
	};
}
