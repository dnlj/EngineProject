// Game
#include <Game/PhysicsComponent.hpp>


namespace Game {
	void PhysicsComponent::setBody(b2Body* body) {
		this->body = body;
	}

	b2Body& PhysicsComponent::getBody() {
		return *body;
	}

	b2World* PhysicsComponent::getWorld() {
		return body->GetWorld();
	}

	void PhysicsComponent::updateTransform(const b2Transform& trans) {
		updateTransform(trans.p, trans.q.GetAngle());
	}

	void PhysicsComponent::updateTransform(const b2Vec2& pos, float32 ang) {
		body->SetTransform(pos, ang);
	}

	void PhysicsComponent::setTransform(const b2Transform& trans) {
		setTransform(trans.p, trans.q.GetAngle());
	}

	void PhysicsComponent::setTransform(const b2Vec2& pos, float32 ang) {
		body->SetTransform(pos, ang);
		prevTransform = body->GetTransform();
		interpTransform = prevTransform;
	}
	
	const b2Vec2& PhysicsComponent::getPosition() const {
		return body->GetPosition();
	}

	const b2Vec2& PhysicsComponent::getInterpPosition() const {
		return interpTransform.p;
	}

	void PhysicsComponent::toNetwork(Engine::Net::Connection& conn) const {
		conn.write(body->GetTransform());
	}

	void PhysicsComponent::fromNetwork(Engine::Net::Connection& conn) {
		const auto trans = conn.read<b2Transform>();
		updateTransform(trans.p, trans.q.GetAngle());
	}
}
