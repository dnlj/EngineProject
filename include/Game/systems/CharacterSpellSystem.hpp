#pragma once

// Box2D
#include <box2d/b2_math.h>

// Game
#include <Game/System.hpp>
#include <Game/PhysicsListener.hpp>


namespace Game {
	class CharacterSpellSystem : public System, public PhysicsListener {
		public:
			struct FireEvent {
				Engine::ECS::Entity ent;
				ZoneId zoneId;
				b2Vec2 pos;
				b2Vec2 dir;
			};

		private:
			std::vector<Engine::ECS::Entity> missiles;
			std::vector<Engine::ECS::Entity> toDestroy;
			std::vector<FireEvent> events;
			size_t currentMissile = 0;

		public:
			CharacterSpellSystem(SystemArg arg);

			void setup();
			void tick();
			void queueMissile(const FireEvent& event);

		private:
			void fireMissile(const FireEvent& event);
			void beginContact(const Engine::ECS::Entity entA, const Engine::ECS::Entity entB) override;
			void detonateMissile(Engine::ECS::Entity ent);
	};
}
