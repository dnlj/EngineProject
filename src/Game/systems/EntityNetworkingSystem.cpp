// Game
#include <Game/World.hpp>
#include <Game/systems/EntityNetworkingSystem.hpp>


namespace Game {
	void EntityNetworkingSystem::tick() {
		if constexpr (ENGINE_CLIENT) { return; }

		using PlayerFilter = Engine::ECS::EntityFilterList<
			Game::PlayerFlag,
			Game::ConnectionComponent
		>;

		// TODO: move into func 
		for (const auto ply : world.getFilter<PlayerFilter>()) {
			auto& neighComp = world.getComponent<NeighborsComponent>(ply);
			auto& physComp = world.getComponent<PhysicsBodyComponent>(ply);
			auto& added = neighComp.addedNeighbors;
			auto& current = neighComp.currentNeighbors;
			auto& removed = neighComp.removedNeighbors;
			{using std::swap; swap(lastNeighbors, current);};
			added.clear();
			current.clear();
			removed.clear();

			struct QueryCallback : b2QueryCallback {
				World& world;
				decltype(current)& ents;
				QueryCallback(World& world, decltype(ents) ents) : world{world}, ents{ents} {}
				virtual bool ReportFixture(b2Fixture* fixture) override {
					const Engine::ECS::Entity ent = Game::PhysicsSystem::toEntity(fixture->GetBody()->GetUserData());
					if (!world.hasComponent<NetworkedFlag>(ent)) { return true; }
					if (!ents.has(ent)) { ents.add(ent); }
					return true;
				}
			} queryCallback(world, current);

			const auto& pos = physComp.getPosition();
			constexpr float32 range = 5; // TODO: what range?
			physComp.getWorld()->QueryAABB(&queryCallback, b2AABB{
				{pos.x - range, pos.y - range},
				{pos.x + range, pos.y + range},
			});

			if (current.has(ply)) {
				current.remove(ply);
			}

			for (const auto lent : lastNeighbors) {
				if (!current.has(lent.first)) {
					ENGINE_LOG("removed: ", lent.first);
					removed.add(lent.first);
				}
			}

			for (const auto cent : current) {
				if (!lastNeighbors.has(cent.first)) {
					ENGINE_LOG("removed: ", cent.first);
					added.add(cent.first);
				}
			}

			// TODO: disabled vs destroyed
		}
	}
}
