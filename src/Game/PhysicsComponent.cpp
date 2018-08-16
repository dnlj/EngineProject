// Game
#include <Game/PhysicsComponent.hpp>


namespace Game {
	PhysicsComponent::PhysicsComponent(PhysicsComponent&& other) {
		// TODO: doesnt swap kinda ruin move?
		swap(*this, other);
	}

	PhysicsComponent::~PhysicsComponent() {
		if (destruct) {
			std::cout << "~PhysicsComponent()\n";

			// TODO: This only needs to be here because we dont have all bodies setup correctly atm
			if (body && physSys) {
				std::cout << "~PhysicsComponent() - destroy\n";
				physSys->destroyBody(body);
			}
		}
	}

	PhysicsComponent& PhysicsComponent::operator=(PhysicsComponent other) {
		swap(*this, other);
		other.destruct = true;
		return *this;
	}

	void swap(PhysicsComponent& first, PhysicsComponent& second) {
		using std::swap;
		swap(first.body, second.body);
		swap(first.physSys, second.physSys);
	}
}
