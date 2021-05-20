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

		// TODO: rm
		//if (world.getAllComponentBitsets().size() > lastCompsBitsets.size()) {
		//	lastCompsBitsets.resize(world.getAllComponentBitsets().size());
		//}

		for (auto& ply : world.getFilter<PlayerFilter>()) {
			auto& ecsNetComp = world.getComponent<ECSNetworkingComponent>(ply);
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
			processAddedNeighbors(ply, conn, ecsNetComp);
			processRemovedNeighbors(ply, conn, ecsNetComp);
			processCurrentNeighbors(ply, conn, ecsNetComp);
		}

		// TODO: rm - lastCompsBitsets = world.getAllComponentBitsets();
	}

	
	template<class C>
	bool EntityNetworkingSystem::networkComponent(const Engine::ECS::Entity ent, Connection& conn) const {
		auto& comp = world.getComponent<C>(ent);
		if (comp.netRepl() == Engine::Net::Replication::NONE) { return true; }

		if (auto msg = conn.beginMessage<MessageType::ECS_COMP_ADD>()) {
			msg.write(ent);
			msg.write(world.getComponentId<C>());
			comp.netToInit(engine, world, ent, msg.getBufferWriter()); // TODO: how to handle with messages? just byte writer?
			return true;
		}

		return false;
	}

	void EntityNetworkingSystem::processAddedNeighbors(const Engine::ECS::Entity ply, Connection& conn, ECSNetworkingComponent& ecsNetComp) {
		for (const auto& [ent, data] : ecsNetComp.neighbors) {
			ENGINE_DEBUG_ASSERT(ent != ply, "A player is not their own neighbor");
			if (data.state != ECSNetworkingComponent::NeighborState::Added) { continue; }

			if (auto msg = conn.beginMessage<MessageType::ECS_ENT_CREATE>()) {
				msg.write(ent);
			}

			// TODO: we should be able to get rid of this and have it handled in processCurrentNeighbors? could use bitflags for state enum to make it easier
			Engine::Meta::ForEachIn<ComponentsSet>::call([&]<class C>() {
				if constexpr (Engine::Net::IsNetworkedComponent<C>) {
					if (!world.hasComponent<C>(ent)) { return; }
					if (!networkComponent<C>(ent, conn)) {
						// TODO: we are hitting this. causing errors
						// TODO: need to handle this case or else replication once doesn't work.
						ENGINE_WARN("Unable network component add. INIT");
					}
				}
			});
		}
	}

	void EntityNetworkingSystem::processRemovedNeighbors(const Engine::ECS::Entity ply, Connection& conn, ECSNetworkingComponent& ecsNetComp) {
		for (const auto& [ent, data] : ecsNetComp.neighbors) {
			ENGINE_DEBUG_ASSERT(ent != ply, "A player is not their own neighbor");
			if (data.state != ECSNetworkingComponent::NeighborState::Removed) { continue; }
			if (auto msg = conn.beginMessage<MessageType::ECS_ENT_DESTROY>()) {
				msg.write(ent);
			}
		}
	}
	
	void EntityNetworkingSystem::processCurrentNeighbors(const Engine::ECS::Entity ply, Connection& conn, ECSNetworkingComponent& ecsNetComp) {
		for (const auto& [ent, data] : ecsNetComp.neighbors) {
			ENGINE_DEBUG_ASSERT(ent != ply, "A player is not their own neighbor");
			if (data.state != ECSNetworkingComponent::NeighborState::Current) { continue; }

			Engine::ECS::ComponentBitset flagComps;

			const auto compsCurr = world.getComponentsBitset(ent);
			// TODO: rm - const auto compsAdd = ~data.comps & compsCurr;
			// TODO: rm - const auto compsRem = data.comps & ~compsCurr;


			Engine::Meta::ForEachIn<ComponentsSet>::call([&]<class C>() {
				// TODO: Note: this only updates components not flags. Still need to network flags.
				constexpr auto cid = world.getComponentId<C>();
				if constexpr (Engine::Net::IsNetworkedComponent<C>) {
					if (!world.hasComponent<C>(ent)) { return; }
					const auto& comp = world.getComponent<C>(ent);

					const auto repl = comp.netRepl();
					if (repl == Engine::Net::Replication::NONE) { return; }

					// TODO: rm - const int32 diff = lastCompsBitsets[ent.id].test(cid) - world.getComponentsBitset(ent).test(cid);
					const int32 diff = data.comps.test(cid) - world.getComponentsBitset(ent).test(cid);

					if (diff < 0) { // Component Added
						ENGINE_LOG("Component added: ", ent, " ", world.getComponentId<C>());

						if (!networkComponent<C>(ent, conn)) {
							// TODO: we really need some whay to handle this.
							// TODO: cont. maybe remove from lastCompsBitsets so it gets sent again next frame?
							// TODO: cont. We really need a lastCompsBitset per connection then.
							ENGINE_WARN("Unable network component add. UPDATE");
						}
					} else if (diff > 0) { // Component Removed
						// TODO: comp removed
					} else if (repl == Engine::Net::Replication::ALWAYS) {
						if (auto msg = conn.beginMessage<MessageType::ECS_COMP_ALWAYS>()) {
							msg.write(ent);
							msg.write(cid);
							if (Engine::ECS::IsSnapshotRelevant<C>::value) {
								msg.write(world.getTick());
							}
									
							comp.netTo(msg.getBufferWriter());
						}
					} else if (repl == Engine::Net::Replication::UPDATE) {
						ENGINE_DEBUG_ASSERT("TODO: Update replication is not yet implemented");
						// TODO: impl
					}
				} else if constexpr (Engine::ECS::IsFlagComponent<C>::value) {
					// TODO: rm - const int32 diff = lastCompsBitsets[ent.id].test(cid) - world.getComponentsBitset(ent).test(cid);
					const int32 diff = data.comps.test(cid) - world.getComponentsBitset(ent).test(cid);
					if (diff) { flagComps.set(cid); }
				}
			});

			if (flagComps) {
				if (auto msg = conn.beginMessage<MessageType::ECS_FLAG>()) {
					msg.write(ent);
					msg.write(flagComps);
				} else {
					// TODO: we either need to always network all flags or have a way to handle this
					// TODO: if we network all flags we probably want a way to tell it to only network certain ones for security/cheat reasons
					ENGINE_WARN("Unable to network entity flags");
				}
			}
		}
	}

	void EntityNetworkingSystem::updateNeighbors() {
		for (const auto ply : world.getFilter<PlayerFilter>()) {
			auto& ecsNetComp = world.getComponent<ECSNetworkingComponent>(ply);
			auto& physComp = world.getComponent<PhysicsBodyComponent>(ply);

			/*
			auto& added = ecsNetComp.addedNeighbors;
			auto& current = ecsNetComp.currentNeighbors;
			auto& removed = ecsNetComp.removedNeighbors;
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
			*/


			ecsNetComp.neighbors.eraseRemove([](auto& pair){
				if (pair.second.state == ECSNetworkingComponent::NeighborState::Removed) {
					return true;
				}

				pair.second.state = ECSNetworkingComponent::NeighborState::Removed;
				return false;
			});


			struct QueryCallback : b2QueryCallback {
				const Engine::ECS::Entity ply;
				World& world;
				ECSNetworkingComponent& ecsNetComp;

				QueryCallback(Engine::ECS::Entity ply, World& world, ECSNetworkingComponent& ecsNetComp)
					: ply{ply}, world{world}, ecsNetComp{ecsNetComp} {
				}

				virtual bool ReportFixture(b2Fixture* fixture) override {
					const Engine::ECS::Entity ent = Game::PhysicsSystem::toEntity(fixture->GetBody()->GetUserData());
					if (!world.hasComponent<NetworkedFlag>(ent)) { return true; }
					if (ecsNetComp.neighbors.has(ent)) {
						ecsNetComp.neighbors.get(ent).state = ECSNetworkingComponent::NeighborState::Current;
					} else if (ent != ply) {
						ecsNetComp.neighbors.add(ent,
							ECSNetworkingComponent::NeighborState::Added
						);
					}
					return true;
				}
			} queryCallback(ply, world, ecsNetComp);

			const auto& pos = physComp.getPosition();
			constexpr float32 range = 5; // TODO: what range?
			physComp.getWorld()->QueryAABB(&queryCallback, b2AABB{
				{pos.x - range, pos.y - range},
				{pos.x + range, pos.y + range},
			});
		}
	}
}
