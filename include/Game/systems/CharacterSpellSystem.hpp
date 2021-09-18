#pragma once

// Box2D
#include <box2d/b2_math.h>

// Game
#include <Game/System.hpp>
#include <Game/PhysicsListener.hpp>


namespace Game {
	class CharacterSpellSystem : public System, public PhysicsListener {
		public:
			CharacterSpellSystem(SystemArg arg);
			void setup();
			void tick();
			void queueMissile(const b2Vec2& pos, const b2Vec2& dir);

		private:
			struct FireEvent {
				Engine::ECS::Entity ent;
				b2Vec2 pos;
				b2Vec2 dir;
			};

			void fireMissile(const b2Vec2& pos, const b2Vec2& dir);
			void beginContact(const Engine::ECS::Entity& entA, const Engine::ECS::Entity& entB) override;
			void detonateMissile(Engine::ECS::Entity ent);
			std::vector<Engine::ECS::Entity> missiles;
			std::vector<Engine::ECS::Entity> toDestroy;
			std::vector<FireEvent> events;
			size_t currentMissile = 0;
	};
}
