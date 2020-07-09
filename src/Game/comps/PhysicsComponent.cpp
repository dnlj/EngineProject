// Game
#include <Game/comps/PhysicsComponent.hpp>
#include <Game/World.hpp>


namespace Game {
	PhysicsComponent::~PhysicsComponent() {
		if (body) {
			body->GetWorld()->DestroyBody(body);
		}
	}

	PhysicsComponent::PhysicsComponent(PhysicsComponent&& other) {
		*this = std::move(other);
	}

	void PhysicsComponent::operator=(PhysicsComponent&& other) {
		using std::swap;
		prevTransform = other.prevTransform;
		interpTransform = other.interpTransform;
		remoteTransform = other.remoteTransform;
		swap(body, other.body); // We need to swap to ensure our old body* gets destroyed
	}

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
		return (body->GetType() == b2_staticBody) ? Engine::Net::Replication::NONE : Engine::Net::Replication::ALWAYS;
	}

	void PhysicsComponent::netTo(Engine::Net::PacketWriter& writer) const {
		writer.write(body->GetTransform());
		writer.write(body->GetLinearVelocity());
	}

	void PhysicsComponent::netToInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Engine::Net::PacketWriter& writer) const {
		netTo(writer);
	}

	void PhysicsComponent::netFrom(Engine::Net::PacketReader& reader) {
		const auto trans = reader.read<b2Transform>();
		updateTransform(trans->p, trans->q.GetAngle());
		//remoteTransform = *trans;

		const auto vel = reader.read<b2Vec2>();
		body->SetLinearVelocity(*vel);
	}

	void PhysicsComponent::netFromInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Engine::Net::PacketReader& reader) {
		auto& physSys = world.getSystem<PhysicsSystem>();
		// TODO: actual shape
		body = physSys.createPhysicsCircle(ent);
		//body->SetType(b2_staticBody);
		netFrom(reader);
	}
}
