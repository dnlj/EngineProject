// Game
#include <Game/comps/PhysicsComponent.hpp>
#include <Game/World.hpp>


namespace Game {
	PhysicsComponent::~PhysicsComponent() {
		// TODO: need to handle when removed from ent: body->setActive(false);
		// TODO: this wont work with copy constructor (rollback buffer)
		// TODO: better way to handle this
		if (body && --*count == 0) {
			body->GetWorld()->DestroyBody(body);
			delete count;
		}
	}
	
	PhysicsComponent::PhysicsComponent(const PhysicsComponent& other) {
		*this = other;
	}

	PhysicsComponent::PhysicsComponent(PhysicsComponent&& other) {
		*this = std::move(other);
	}

	void PhysicsComponent::operator=(const PhysicsComponent& other) {
		storedTransform = other.storedTransform;
		storedVelocity = other.storedVelocity;
		storedAngularVelocity = other.storedAngularVelocity;
		prevTransform = other.prevTransform;
		interpTransform = other.interpTransform;
		remoteTransform = other.remoteTransform;
		body = other.body;
		count = other.count;
		++*count;
	}

	void PhysicsComponent::operator=(PhysicsComponent&& other) {
		using std::swap;
		storedTransform = other.storedTransform;
		storedVelocity = other.storedVelocity;
		storedAngularVelocity = other.storedAngularVelocity;
		prevTransform = other.prevTransform;
		interpTransform = other.interpTransform;
		remoteTransform = other.remoteTransform;
		count = other.count;
		++*count;
		swap(body, other.body); // We need to swap to ensure our old body* gets destroyed
	}

	void PhysicsComponent::setBody(b2Body* body) {
		this->body = body;
		ENGINE_DEBUG_ASSERT(count == nullptr);

		count = new int{1};
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
		updateTransform(pos, ang);
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
		updateTransform(*reader.read<b2Transform>());
		body->SetLinearVelocity(*reader.read<b2Vec2>());
	}

	void PhysicsComponent::netFromInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Engine::Net::PacketReader& reader) {
		auto& physSys = world.getSystem<PhysicsSystem>();
		// TODO: actual shape
		setBody(physSys.createPhysicsCircle(ent));
		//body->SetType(b2_staticBody);
		netFrom(reader);
	}
}
