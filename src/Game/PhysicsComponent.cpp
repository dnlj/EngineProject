// Game
#include <Game/PhysicsComponent.hpp>


namespace Game {
	void PhysicsComponent::setBody(b2Body* body) {
		this->body = body;
	}

	b2Body& PhysicsComponent::getBody() {
		return *body;
	}

	void PhysicsComponent::setTransform(const b2Vec2& pos, float32 ang) {
		body->SetTransform(pos, ang);
		prevTransform.Set(pos, ang);
		interpTransform.Set(pos, ang);
	}
	
	const b2Vec2& PhysicsComponent::getPosition() const {
		return body->GetPosition();
	}

	const b2Vec2& PhysicsComponent::getInterpPosition() const {
		return interpTransform.p;
	}
}
