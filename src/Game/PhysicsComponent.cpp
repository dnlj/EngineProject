// Game
#include <Game/PhysicsComponent.hpp>


namespace Game {
	PhysicsComponent::PhysicsComponent(PhysicsComponent&& other) {
		// TODO: doesnt swap kinda ruin move?
		swap(*this, other);
	}

	PhysicsComponent::~PhysicsComponent() {
		if (destruct) {
			physSys->destroyBody(body);
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
