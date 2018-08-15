#pragma once

// Box2D
#include <Box2D/Box2D.h>

// Game
#include <Game/PhysicsSystem.hpp>


namespace Game {
	class PhysicsComponent {
		public:
			// TODO: Move
			friend void swap(PhysicsComponent& first, PhysicsComponent& second) {
				using std::swap;
				swap(first.body, second.body);
				swap(first.physSys, second.physSys);
			}

			PhysicsComponent() = default;
			~PhysicsComponent();

			// TODO: Move
			PhysicsComponent& operator=(PhysicsComponent other) {
				swap(*this, other);
				return *this;
			}

			// TODO: Move
			PhysicsComponent(PhysicsComponent&& other) {
				other.destruct = false;

				// TODO: doesnt swap kinda ruin move?
				swap(*this, other);
			}

			b2Body* body = nullptr;
			PhysicsSystem* physSys = nullptr;

		private:
			bool destruct = true;
	};
}
