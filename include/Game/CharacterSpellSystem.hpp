#pragma once

// Box2D
#include <box2d/b2_math.h>

// Engine
#include <Engine/EngineInstance.hpp>

// Game
#include <Game/System.hpp>
#include <Game/PhysicsListener.hpp>


namespace Game {
	class CharacterSpellSystem : public System, public PhysicsListener {
		public:
			CharacterSpellSystem(SystemArg arg);
			void setup(Engine::EngineInstance& engine);
			void run(float dt);
			void fireMissile(const b2Vec2& pos, const b2Vec2& dir);

		private:
			void beginContact(const PhysicsUserData& dataA, const PhysicsUserData& dataB) override;
			void detonateMissle(Engine::ECS::Entity ent);

			std::vector<Engine::ECS::Entity> missles;
			std::vector<Engine::ECS::Entity> toDestroy;
			size_t currentMissle = 0;
	};
}
