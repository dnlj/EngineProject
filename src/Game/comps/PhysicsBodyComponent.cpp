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
		trans = other.trans;
		vel = other.vel;
		angVel = other.angVel;
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
		trans = other.trans;
		vel = other.vel;
		angVel = other.angVel;
	}

	void PhysicsBodyComponent::setBody(b2Body* body) {
		this->body = body;
		ENGINE_DEBUG_ASSERT(count == nullptr);
		count = new int{1};
		fromBody();
	}

	
	void PhysicsBodyComponent::toBody() {
		const auto& btrans = body->GetTransform();

		if (abs(trans.p.x) > 10'000) { // TODO: rm
			__debugbreak();
		}

		// Some reason b2Transform doesnt have comparison operators?
		if (trans.p != btrans.p || trans.q.c != btrans.q.c || trans.q.s != btrans.q.s) {
			setTransform(trans.p, trans.q.GetAngle());
		}

		if (vel != body->GetLinearVelocity()) {
			setVelocity(vel);
		}

		if (angVel != body->GetAngularVelocity()) {
			setAngularVelocity(angVel);
		}
	}

	void PhysicsBodyComponent::fromBody() {
		if (abs(trans.p.x) > 10'000) { // TODO: rm
			__debugbreak();
		}

		trans = body->GetTransform();
		vel = body->GetLinearVelocity();
		angVel = body->GetAngularVelocity();

		if (abs(trans.p.x) > 10'000) { // TODO: rm
			__debugbreak();
		}
	}

	
	void PhysicsBodyComponent::netTo(Engine::Net::BufferWriter& buff) const {
		buff.write(trans);
		buff.write(vel);
	}

	void PhysicsBodyComponent::netToInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Engine::Net::BufferWriter& buff) const {
		buff.write(type);
		netTo(buff);
	}

	void PhysicsBodyComponent::netFrom(Connection& conn) {
		trans = *conn.read<b2Transform>();
		vel = *conn.read<b2Vec2>();
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
