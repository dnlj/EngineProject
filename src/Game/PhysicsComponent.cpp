// Game
#include <Game/PhysicsComponent.hpp>
#include <Game/World.hpp>


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
		return (body->GetType() == b2_staticBody) ? Engine::Net::Replication::UPDATE : Engine::Net::Replication::ALWAYS;
	}

	void PhysicsComponent::netTo(Engine::Net::PacketWriter& writer) const {
		writer.write(body->GetTransform());
		std::cout << "Write: " << body->GetTransform().p.x << " " << body->GetTransform().p.y << "\n";
	}

	void PhysicsComponent::netToInit(World& world, Engine::ECS::Entity ent, Engine::Net::PacketWriter& writer) const {
		netTo(writer);
	}

	void PhysicsComponent::netFrom(Engine::Net::PacketReader& reader) {
		const auto trans = reader.read<b2Transform>();
		// TODO: why doesnt update just take a transform?
		updateTransform(trans->p, trans->q.GetAngle());
		std::cout << "Read: " << trans->p.x << " " << trans->p.y << "\n";
	}

	void PhysicsComponent::netFromInit(World& world, Engine::ECS::Entity ent, Engine::Net::PacketReader& reader) {
		auto& physSys = world.getSystem<PhysicsSystem>();
		// TODO: actual shape
		body = physSys.createPhysicsCircle(ent);
		netFrom(reader);
	}
}
