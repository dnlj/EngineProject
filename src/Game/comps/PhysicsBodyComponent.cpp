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

	b2Body& PhysicsBodyComponent::getBody() {
		return *body;
	}

	b2World* PhysicsBodyComponent::getWorld() {
		return body->GetWorld();
	}

	void PhysicsBodyComponent::netFromInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Connection& conn) {
		auto& physSys = world.getSystem<PhysicsSystem>();
		setBody(physSys.createPhysicsCircle(ent));
		netFrom(conn);
	}
}
