// Game
#include <Game/comps/PhysicsComponent.hpp>
#include <Game/World.hpp>


namespace Game {
	PhysicsComponent::~PhysicsComponent() noexcept {
		// TODO: need to handle when removed from ent: body->setActive(false);
		// TODO: this wont work with copy constructor (rollback buffer)
		// TODO: better way to handle this

		if (count && --*count == 0) {
			ENGINE_DEBUG_ASSERT(body, "Incorrect reference counting. PhysicsComponent::body was nullptr.");
			body->GetWorld()->DestroyBody(body);
			delete count;
		}
	}
	
	PhysicsComponent::PhysicsComponent(const PhysicsComponent& other) noexcept {
		stored = other.stored;
		count = other.count;
		body = other.body;
		if (count) { ++*count; }
	}

	PhysicsComponent::PhysicsComponent(PhysicsComponent&& other) noexcept {
		*this = std::move(other);
	}

	void PhysicsComponent::operator=(const PhysicsComponent& other) noexcept {
		auto copy{other};
		*this = std::move(copy);
	}

	void PhysicsComponent::operator=(PhysicsComponent&& other) noexcept {
		using std::swap;
		stored = other.stored;
		swap(count, other.count);
		swap(body, other.body);
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

	Engine::Net::Replication PhysicsComponent::netRepl() const {
		return (body->GetType() == b2_staticBody) ? Engine::Net::Replication::NONE : Engine::Net::Replication::ALWAYS;
	}

	void PhysicsComponent::netTo(Connection& conn) const {
		conn.write(body->GetTransform());
		conn.write(body->GetLinearVelocity());
	}

	void PhysicsComponent::netToInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Connection& conn) const {
		netTo(conn);
	}

	void PhysicsComponent::netFrom(Connection& conn) {
		setTransform2(*conn.read<b2Transform>());
		body->SetLinearVelocity(*conn.read<b2Vec2>());
		//conn.read<b2Transform>();
		//conn.read<b2Vec2>();
	}

	void PhysicsComponent::netFromInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Connection& conn) {
		auto& physSys = world.getSystem<PhysicsSystem>();
		// TODO: actual shape
		setBody(physSys.createPhysicsCircle(ent));
		//body->SetType(b2_staticBody);
		netFrom(conn);
	}
}
