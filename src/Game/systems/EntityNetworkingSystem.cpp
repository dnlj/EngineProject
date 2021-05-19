// STD
#include <chrono>

// Engine
#include <Engine/Meta/ForEach.hpp>

// Game
#include <Game/World.hpp>
#include <Game/systems/EntityNetworkingSystem.hpp>


namespace {
	using PlayerFilter = Engine::ECS::EntityFilterList<
		Game::PlayerFlag,
		Game::ConnectionComponent
	>;
}

namespace Game {
	void EntityNetworkingSystem::run(float32 dt) {
		if constexpr (ENGINE_CLIENT) { return; }
		const auto now = Engine::Clock::now();
		if (now < nextUpdate) { return; }

		// TODO: config for this
		nextUpdate = now + std::chrono::milliseconds{1000 / 20};

		updateNeighbors();

		if (world.getAllComponentBitsets().size() > lastCompsBitsets.size()) {
			lastCompsBitsets.resize(world.getAllComponentBitsets().size());
		}

		for (auto& ply : world.getFilter<PlayerFilter>()) {
			auto& neighComp = world.getComponent<NeighborsComponent>(ply);
			auto& conn = *world.getComponent<ConnectionComponent>(ply).conn;

			// TODO: move elsewhere, this isnt really related to ECS networking
			{ // TODO: player data should be sent every tick along with actions/inputs.
			// TODO: cont.  Should it? every few frames is probably fine for keeping it in sync. Although when it does desync it will be a larger rollback.
				auto& physComp = world.getComponent<PhysicsBodyComponent>(ply);
				if (auto msg = conn.beginMessage<MessageType::PLAYER_DATA>()) {
					msg.write(world.getTick() + 1); // since this is in `run` and not before `tick` we are sending one tick off. +1 is temp fix
					msg.write(physComp.getTransform());
					msg.write(physComp.getVelocity());
				}
			}

			// TODO: handle entities without phys comp?
			// TODO: figure out which entities have been updated
			// TODO: prioritize entities
			// TODO: figure out which comps on those entities have been updated
			processAddedNeighbors(ply, conn, neighComp);
			processRemovedNeighbors(ply, conn, neighComp);
			processCurrentNeighbors(ply, conn, neighComp);
		}

		lastCompsBitsets = world.getAllComponentBitsets();
	}

	void EntityNetworkingSystem::processAddedNeighbors(const Engine::ECS::Entity ply, Connection& conn, NeighborsComponent& neighComp) {
		for (const auto& pair : neighComp.addedNeighbors) {
			ENGINE_DEBUG_ASSERT(pair.first != ply, "A player is not their own neighbor");
			const auto& ent = pair.first;

			if (auto msg = conn.beginMessage<MessageType::ECS_ENT_CREATE>()) {
				msg.write(ent);
			}

			Engine::Meta::ForEachIn<ComponentsSet>::call([&]<class C>() {
				if constexpr (Engine::Net::IsNetworkedComponent<C>) {
					if (!world.hasComponent<C>(ent)) { return; }

					auto& comp = world.getComponent<C>(ent);
					if (comp.netRepl() == Engine::Net::Replication::NONE) { return; }

					if (auto msg = conn.beginMessage<MessageType::ECS_COMP_ADD>()) {
						msg.write(ent);
						msg.write(world.getComponentId<C>());
							
						comp.netToInit(engine, world, ent, msg.getBufferWriter()); // TODO: how to handle with messages? just byte writer?
					}
				}
			});
		}
	}

	void EntityNetworkingSystem::processRemovedNeighbors(const Engine::ECS::Entity ply, Connection& conn, NeighborsComponent& neighComp) {
		for (const auto& pair : neighComp.removedNeighbors) {
			ENGINE_DEBUG_ASSERT(pair.first != ply, "A player is not their own neighbor");
			if (auto msg = conn.beginMessage<MessageType::ECS_ENT_DESTROY>()) {
				msg.write(pair.first);
			}
		}
	}
	
	void EntityNetworkingSystem::processCurrentNeighbors(const Engine::ECS::Entity ply, Connection& conn, NeighborsComponent& neighComp) {
		for (const auto& pair : neighComp.currentNeighbors) {
			ENGINE_DEBUG_ASSERT(pair.first != ply, "A player is not their own neighbor");
			const auto ent = pair.first;
			Engine::ECS::ComponentBitset flagComps;

			Engine::Meta::ForEachIn<ComponentsSet>::call([&]<class C>() {
				// TODO: Note: this only updates components not flags. Still need to network flags.
				constexpr auto cid = world.getComponentId<C>();
				if constexpr (Engine::Net::IsNetworkedComponent<C>) {
					if (!world.hasComponent<C>(ent)) { return; }
					const auto& comp = world.getComponent<C>(ent);
					const auto repl = comp.netRepl();
					const int32 diff = lastCompsBitsets[ent.id].test(cid) - world.getComponentsBitset(ent).test(cid);

					// TODO: repl then diff. not diff then repl

					if (diff < 0) { // Component Added
						// TODO: comp added
						// TODO: currently this is duplicate with addedNeighbors init
						//conn.writer.next(MessageType::ECS_COMP_ADD, Engine::Net::Channel::ORDERED);
						//conn.writer.write(cid);
						//comp.netToInit(world, ent, conn.writer);
					} else if (diff > 0) { // Component Removed
						// TODO: comp removed
					} else { // Component Updated
						if (repl == Engine::Net::Replication::ALWAYS) {
							if (auto msg = conn.beginMessage<MessageType::ECS_COMP_ALWAYS>()) {
								msg.write(ent);
								msg.write(cid);
								if (Engine::ECS::IsSnapshotRelevant<C>::value) {
									msg.write(world.getTick());
								}
									
								comp.netTo(msg.getBufferWriter());
							}
						} else if (repl == Engine::Net::Replication::UPDATE) {
							// TODO: impl
						}
					}
				} else if constexpr (Engine::ECS::IsFlagComponent<C>::value) {
					const int32 diff = lastCompsBitsets[ent.id].test(cid) - world.getComponentsBitset(ent).test(cid);
					if (diff) {
						flagComps.set(cid);
					}
				}
			});

			if (flagComps) {
				if (auto msg = conn.beginMessage<MessageType::ECS_FLAG>()) {
					msg.write(ent);
					msg.write(flagComps);
				}
			}
			}
	}

	void EntityNetworkingSystem::updateNeighbors() {
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
					ENGINE_LOG("added: ", cent.first);
					added.add(cent.first);
				}
			}

			// TODO: disabled vs destroyed
		}
	}
}
