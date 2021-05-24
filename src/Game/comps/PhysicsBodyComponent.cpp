// Game
#include <Game/comps/PhysicsBodyComponent.hpp>
#include <Game/World.hpp>


namespace Game {
	PhysicsBodyComponent::~PhysicsBodyComponent() noexcept {
		// TODO: need to handle when removed from ent: body->setActive(false);
		// TODO: this wont work with copy constructor (rollback buffer)
		// TODO: better way to handle this

		if (count && --*count == 0) {
			ENGINE_DEBUG_ASSERT(body, "Incorrect reference counting. PhysicsBodyComponent::body was nullptr.");
			body->GetWorld()->DestroyBody(body);
			delete count;
		}
	}
	
	PhysicsBodyComponent::PhysicsBodyComponent(const PhysicsBodyComponent& other) noexcept {
		count = other.count;
		body = other.body;
		if (count) { ++*count; }
	}

	PhysicsBodyComponent::PhysicsBodyComponent(PhysicsBodyComponent&& other) noexcept {
		*this = std::move(other);
	}

	void PhysicsBodyComponent::operator=(const PhysicsBodyComponent& other) noexcept {
		auto copy{other};
		*this = std::move(copy);
	}

	void PhysicsBodyComponent::operator=(PhysicsBodyComponent&& other) noexcept {
		using std::swap;
		swap(count, other.count);
		swap(body, other.body);
	}

	void PhysicsBodyComponent::setBody(b2Body* body) {
		this->body = body;
		ENGINE_DEBUG_ASSERT(count == nullptr);
		count = new int{1};
	}

	void PhysicsBodyComponent::netTo(Engine::Net::BufferWriter& buff) const {
		buff.write(getTransform());
		buff.write(getVelocity());
	}

	void PhysicsBodyComponent::netToInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Engine::Net::BufferWriter& buff) const {
		buff.write(type);
		netTo(buff);
	}

	void PhysicsBodyComponent::netFrom(Connection& conn) {
		const auto trans = *conn.read<b2Transform>();
		const auto vel = *conn.read<b2Vec2>();
		setTransform(trans.p, trans.q.GetAngle());
		rollbackOverride = true;
	}

	void PhysicsBodyComponent::netFromInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Connection& conn) {
		const auto* type = conn.read<PhysicsType>();
		if (!type) {
			ENGINE_WARN("Unable to read physics type from network.");
			return;
		}

		auto& physSys = world.getSystem<PhysicsSystem>();
		setBody(physSys.createPhysicsCircle(ent, {}, -+*type));
		this->type = *type;

		netFrom(conn);
	}
}
