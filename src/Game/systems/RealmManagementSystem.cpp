#if ENGINE_SERVER

// Game
#include <Game/comps/PhysicsBodyComponent.hpp>
#include <Game/comps/RealmComponent.hpp>
#include <Game/systems/PhysicsSystem.hpp>
#include <Game/systems/RealmManagementSystem.hpp>
#include <Game/systems/ZoneManagementSystem.hpp>


namespace Game {
	RealmManagementSystem::RealmManagementSystem(SystemArg arg) : System{arg} {
		static_assert(World::orderBefore<RealmManagementSystem, ZoneManagementSystem>());
		//static_assert(World::orderBefore<PhysicsSystem, ZoneManagementSystem>());
	}

	void RealmManagementSystem::setup() {
		auto& physSys = world.getSystem<PhysicsSystem>();
		physSys.addListener(this);
	}

	void RealmManagementSystem::tick() {
		ENGINE_DEBUG_ASSERT(ENGINE_SERVER, "This system should only be run server side.");

		for (const auto& change : changes) {
			ENGINE_INFO2("Realm Change: {} {} {}", change.ply, change.realmId, change.pos);
			auto& zoneSys = world.getSystem<ZoneManagementSystem>();
			zoneSys.movePlayer(change.ply, change.realmId, change.pos);
		}

		changes.clear();
	}
	
	void RealmManagementSystem::beginContact(const Engine::ECS::Entity entA, const Engine::ECS::Entity entB) {
		Engine::ECS::Entity ply;
		Engine::ECS::Entity portal;

		if (world.hasComponent<PlayerFlag>(entA) && world.hasComponent<RealmComponent>(entB)) {
			ply = entA;
			portal = entB;
		} else if (world.hasComponent<PlayerFlag>(entB) && world.hasComponent<RealmComponent>(entA)) {
			ply = entB;
			portal = entA;
		} else {
			return;
		}

		ENGINE_INFO2("Realm: {} {}", ply, portal);
		const auto& realmComp = world.getComponent<RealmComponent>(portal);
		changes.push_back({
			.ply = ply,
			.realmId = realmComp.realmId,
			.pos = realmComp.pos,
		});
	}
}
#endif
