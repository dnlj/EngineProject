#pragma once

// Box2D
#include <Box2D/Box2D.h>

// Engine
#include <Engine/Debug/DebugDrawBox2D.hpp>

// Game
#include <Game/Common.hpp>


namespace Game {
	class PhysicsSystem : public SystemBase {
		public:
			PhysicsSystem(World& world);
			virtual void run(float dt) override;

			b2World& getPhysicsWorld();

		private:
			b2World physWorld;

			#if defined(DEBUG_PHYSICS)
				Engine::Debug::DebugDrawBox2D debugDraw;
			#endif
	};
}
