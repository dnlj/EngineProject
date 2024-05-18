// Game
#include <Game/systems/PhysicsSystem.hpp>
#include <Game/systems/RealmManagementSystem.hpp>


namespace Game {
	RealmManagementSystem::RealmManagementSystem(SystemArg arg) : System{arg} {
	}

	void RealmManagementSystem::setup() {
		auto& physSys = world.getSystem<PhysicsSystem>();
		physSys.addListener(this);
	}

	void RealmManagementSystem::tick() {
		for (const auto& change : changes) {
			// TODO:
			change;
		}

		changes.clear();
	}
	
	void RealmManagementSystem::beginContact(const Engine::ECS::Entity entA, const Engine::ECS::Entity entB) {
		if (world.hasComponent<PlayerFlag>(entA) && world.hasComponent<RealmComponent>(entB)) {
			ENGINE_INFO2("Realm: {} {}", entA, entB);
		} else if (world.hasComponent<PlayerFlag>(entB) && world.hasComponent<RealmComponent>(entA)) {
			ENGINE_INFO2("Realm: {} {}", entB, entA);
		}
	}
}
