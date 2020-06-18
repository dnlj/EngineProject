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
	
	Engine::Net::Replication PhysicsComponent::netRepl() const {
		return body->GetType() == b2_staticBody ? Engine::Net::Replication::UPDATE : Engine::Net::Replication::ALWAYS;
	}

	void PhysicsComponent::netTo(Engine::Net::Connection& conn) const {
		conn.writer.write(body->GetTransform());
	}

	void PhysicsComponent::netFrom(Engine::Net::Connection& conn) {
		const auto trans = conn.reader.read<b2Transform>();
		updateTransform(trans->p, trans->q.GetAngle());
	}
}
