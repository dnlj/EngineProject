// STD
#include <chrono>

// Game
#include <Game/World.hpp>
#include <Game/systems/EntityNetworkingSystem.hpp>


namespace {
	using PlayerFilter = Engine::ECS::EntityFilterList<
		Game::PlayerFlag,
		Game::ConnectionComponent
	>;

	// TODO: move to central location. Copied from networkingsystem.cpp
	template<class T>
	concept IsNetworkedComponent = requires (T t) {
		Engine::Net::Replication{t.netRepl()};
	};

	// TODO: move into own class
	// TODO: add version for Ts... and unpack<Set>
	template<class ComponentsSet>
	struct ForEachIn {
		template<class Func>
		static void call(Func&& func) {};
	};

	template<template<class...> class ComponentsSet, class... Components>
	struct ForEachIn<ComponentsSet<Components...>> {
		template<class Func>
		static void call(Func&& func) {
			(func.operator()<Components>(), ...);
		}
	};
}

namespace Game {
	void EntityNetworkingSystem::tick() {
		if constexpr (ENGINE_CLIENT) { return; }
		updateNeighbors();
	}

	void EntityNetworkingSystem::run(float32 dt) {
		if constexpr (ENGINE_CLIENT) { return; }


		const auto now = Engine::Clock::now();
		if (nextUpdate < now) { return; }

		// TODO: config for this
		nextUpdate = now + std::chrono::milliseconds{1000 / 20};

		if (world.getAllComponentBitsets().size() > lastCompsBitsets.size()) {
			lastCompsBitsets.resize(world.getAllComponentBitsets().size());
		}

		for (auto& ply : world.getFilter<PlayerFilter>()) {
			auto& neighComp = world.getComponent<NeighborsComponent>(ply);
			auto& conn = *world.getComponent<ConnectionComponent>(ply).conn;

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

			for (const auto& pair : neighComp.addedNeighbors) {
				ENGINE_DEBUG_ASSERT(pair.first != ply, "A player is not their own neighbor");
				const auto& ent = pair.first;

				if (auto msg = conn.beginMessage<MessageType::ECS_ENT_CREATE>()) { // TODO: General_RO;
					msg.write(ent);
				}

				ForEachIn<ComponentsSet>::call([&]<class C>() {
					if constexpr (IsNetworkedComponent<C>) {
						ENGINE_LOG("IsNetworkedComponent ", world.getComponentId<C>());
						if (!world.hasComponent<C>(ent)) { return; }

						auto& comp = world.getComponent<C>(ent);
						if (comp.netRepl() == Engine::Net::Replication::NONE) { return; }

						if (auto msg = conn.beginMessage<MessageType::ECS_COMP_ADD>()) {//, General_RO);
							msg.write(ent);
							msg.write(world.getComponentId<C>());
							
							comp.netToInit(engine, world, ent, msg.getBufferWriter()); // TODO: how to handle with messages? just byte writer?
						}
					}
				});
			}

			for (const auto& pair : neighComp.removedNeighbors) {
				ENGINE_DEBUG_ASSERT(pair.first != ply, "A player is not their own neighbor");
				if (auto msg = conn.beginMessage<MessageType::ECS_ENT_DESTROY>()) { // TODO: General_RO;
					msg.write(pair.first);
				}
			}

			for (const auto& pair : neighComp.currentNeighbors) {
				ENGINE_DEBUG_ASSERT(pair.first != ply, "A player is not their own neighbor");
				const auto ent = pair.first;
				Engine::ECS::ComponentBitset flagComps;

				ForEachIn<ComponentsSet>::call([&]<class C>() {
					// TODO: Note: this only updates components not flags. Still need to network flags.
					constexpr auto cid = world.getComponentId<C>();
					if constexpr (IsNetworkedComponent<C>) {
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
					if (auto msg = conn.beginMessage<MessageType::ECS_FLAG>()) { // TODO: General_RO
						msg.write(ent);
						msg.write(flagComps);
					}
				}
			}
		}

		lastCompsBitsets = world.getAllComponentBitsets();
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
					ENGINE_LOG("removed: ", cent.first);
					added.add(cent.first);
				}
			}

			// TODO: disabled vs destroyed
		}
	}
}
