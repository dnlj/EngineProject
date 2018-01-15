#pragma once

// Box2D
#include <Box2D/Box2D.h>

namespace Game {
	class PhysicsComponent {
		public:
			b2Body* body = nullptr;
			void setup(b2World& world);
	};
}
