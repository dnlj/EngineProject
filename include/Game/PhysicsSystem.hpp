#pragma once

// Box2D
#include <Box2D/Box2D.h>

// Engine
#include <Engine/Debug/DebugDrawBox2D.hpp>

// Game
#include <Game/Common.hpp>
#include <Game/PhysicsUserData.hpp>


namespace Game {
	// TODO: Move to file
	class TestContactListener : public b2ContactListener {
		public:
			void BeginContact(b2Contact* contact) override {
				const auto dataA = static_cast<PhysicsUserData*>(contact->GetFixtureA()->GetBody()->GetUserData());
				const auto dataB = static_cast<PhysicsUserData*>(contact->GetFixtureB()->GetBody()->GetUserData());

				if (dataA == nullptr) { return; }
				if (dataB == nullptr) { return; }

				std::cout
					<< "A: "<< dataA << " "
					<< "B: " << dataB << "\n";
			}
	};

	class PhysicsSystem : public SystemBase {
		public:
			PhysicsSystem(World& world);

			virtual void run(float dt) override;

			b2Body* createBody(Engine::ECS::Entity ent, b2BodyDef& bodyDef);

			b2World& getPhysicsWorld();

			#if defined(DEBUG_PHYSICS)
				Engine::Debug::DebugDrawBox2D& getDebugDraw();
			#endif

		private:
			b2World physWorld;
			TestContactListener contactListener;

			std::vector<PhysicsUserData> userData;

			#if defined(DEBUG_PHYSICS)
				Engine::Debug::DebugDrawBox2D debugDraw;
			#endif
	};
}
