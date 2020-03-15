// Game
#include <Game/PhysicsComponent.hpp>


namespace Game {
	void PhysicsComponent::setTransform(const b2Vec2& pos, float32 ang) {
		body->SetTransform(pos, ang);
		prevTransform.Set(pos, ang);
		interpTransform.Set(pos, ang);
	}

	const b2Vec2& PhysicsComponent::getInterpPosition() const {
		return interpTransform.p;
	}
}
