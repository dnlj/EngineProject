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
	

	class PhysicsSystem : public SystemBase {
		public:
			PhysicsSystem(World& world);

			virtual void run(float dt) override;


			b2Body* createBody(Engine::ECS::Entity ent, b2BodyDef& bodyDef);

			const PhysicsUserData& getUserData(void* ptr) const;

			b2World& getPhysicsWorld();

			#if defined(DEBUG_PHYSICS)
				Engine::Debug::DebugDrawBox2D& getDebugDraw();
			#endif

		private:
			class ContactListener : public b2ContactListener {
				public:
					ContactListener(PhysicsSystem& physSys);
					void BeginContact(b2Contact* contact) override;

				private:
					PhysicsSystem& physSys;
			};

			b2World physWorld;

			ContactListener contactListener;

			std::vector<PhysicsUserData> userData;

			#if defined(DEBUG_PHYSICS)
				Engine::Debug::DebugDrawBox2D debugDraw;
			#endif
	};
}
