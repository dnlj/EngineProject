// Game
#include <Game/PhysicsComponent.hpp>


namespace Game {
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
}
