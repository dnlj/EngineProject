#pragma once

// Box2D
#include <Box2D/Box2D.h>

// Game
#include <Game/PhysicsSystem.hpp>


namespace Game {
	class PhysicsComponent {
		public:
			PhysicsComponent() = default;
			PhysicsComponent(PhysicsComponent&& other);
			~PhysicsComponent();
			PhysicsComponent& operator=(PhysicsComponent other);

			friend void swap(PhysicsComponent& first, PhysicsComponent& second);

			b2Body* body = nullptr;
			PhysicsSystem* physSys = nullptr;

		private:
			bool destruct = true;
	};
}
