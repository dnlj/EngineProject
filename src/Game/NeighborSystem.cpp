// Game
#include <Game/NeighborSystem.hpp>
#include <Game/World.hpp>

namespace Game {
	NeighborSystem::NeighborSystem(SystemArg arg)
		: System{arg}
		, neighborFilter{world.getFilterFor<NeighborsComponent>()} {
	}

	void NeighborSystem::tick(float32 dt) {
		for (const auto ent : neighborFilter) {
			auto& neighComp = world.getComponent<NeighborsComponent>(ent);
			auto& physComp = world.getComponent<PhysicsComponent>(ent);
			auto& added = neighComp.addedNeighbors;
			auto& current = neighComp.currentNeighbors;
			auto& removed = neighComp.removedNeighbors;
			{using std::swap; swap(last, current);};
			added.clear();
			current.clear();
			removed.clear();

			struct QueryCallback : b2QueryCallback {
				decltype(current)& ents;
				World& world;
				QueryCallback(World& world, decltype(ents) ents) : world{world}, ents{ents} {}
				virtual bool ReportFixture(b2Fixture* fixture) override {
					const Engine::ECS::Entity ent = Game::PhysicsSystem::toEntity(fixture->GetBody()->GetUserData());
					if (!ents.has(ent)) {
						ents.add(ent);
					}
					return true;
				}
			} queryCallback(world, current);

			const auto& pos = physComp.getPosition();
			constexpr float32 range = 5; // TODO: what range?
			physComp.getWorld()->QueryAABB(&queryCallback, b2AABB{
				{pos.x - range, pos.y - range},
				{pos.x + range, pos.y + range},
			});

			for (const auto lent : last) {
				if (!current.has(lent.first)) {
					removed.add(lent.first);
				}
			}

			for (const auto cent : current) {
				if (!last.has(cent.first)) {
					added.add(cent.first);
				}
			}

			// TODO: disabled vs destroyed
		}
	}
}
