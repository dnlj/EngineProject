// STD
#include <chrono>

// Engine
#include <Engine/Meta/ForEach.hpp>

// Game
#include <Game/World.hpp>
#include <Game/systems/EntityNetworkingSystem.hpp>
#include <Game/comps/all.hpp>


namespace {
	using PlayerFilter = Engine::ECS::EntityFilterList<
		Game::PlayerFlag,
		Game::ConnectionComponent
	>;
}

namespace Game {
	void EntityNetworkingSystem::update(float32 dt) {
		if constexpr (ENGINE_CLIENT) { return; }
		const auto now = world.getTime();
		if (now < nextUpdate) { return; }

		// TODO: config for this
		nextUpdate = now + std::chrono::milliseconds{1000 / 20};

		updateNeighbors();

		for (auto& ply : world.getFilter<PlayerFilter>()) {
			auto& ecsNetComp = world.getComponent<ECSNetworkingComponent>(ply);
			auto& conn = *world.getComponent<ConnectionComponent>(ply).conn;

			// TODO: move elsewhere, this isnt really related to ECS networking
			{ // TODO: player data should be sent every tick along with actions/inputs.
			// TODO: cont.  Should it? every few frames is probably fine for keeping it in sync. Although when it does desync it will be a larger rollback.
				const auto& physComp = world.getComponent<PhysicsBodyComponent>(ply);
				if (auto msg = conn.beginMessage<MessageType::PLAYER_DATA>()) {
					msg.write(world.getTick() + 1); // since this is in `run` and not before `tick` we are sending one tick off. +1 is temp fix
					msg.write(physComp.getTransform());
					msg.write(physComp.getVelocity());
				}
			}

			// TODO: handle entities without phys comp?
			// TODO: figure out which entities have been updated
			// TODO: prioritize entities

			// Order is important here since some failed writes change neighbor states
			processAddedNeighbors(ply, conn, ecsNetComp);
			processRemovedNeighbors(ply, conn, ecsNetComp);
			processCurrentNeighbors(ply, conn, ecsNetComp);
		}
	}

	
	template<class C>
	bool EntityNetworkingSystem::networkComponent(const Engine::ECS::Entity ent, Connection& conn) const {
		auto& comp = world.getComponent<C>(ent);
		if (NetworkTraits<C>::getReplType(comp) == Engine::Net::Replication::NONE) { return true; }

		if (auto msg = conn.beginMessage<MessageType::ECS_COMP_ADD>()) {
			msg.write(ent);
			msg.write(world.getComponentId<C>());
			NetworkTraits<C>::writeInit(comp, msg.getBufferWriter(), engine, world, ent);
			return true;
		}

		return false;
	}

	void EntityNetworkingSystem::processAddedNeighbors(const Engine::ECS::Entity ply, Connection& conn, ECSNetworkingComponent& ecsNetComp) {
		for (auto& [ent, data] : ecsNetComp.neighbors) {
			ENGINE_DEBUG_ASSERT(ent != ply, "A player is not their own neighbor");
			if (data.state != ECSNetworkingComponent::NeighborState::Added) { continue; }

			if (auto msg = conn.beginMessage<MessageType::ECS_ENT_CREATE>()) {
				msg.write(ent);
			} else {
				data.state = ECSNetworkingComponent::NeighborState::None;
				ENGINE_WARN("Unable to network entity create.");
			}
		}
	}

	void EntityNetworkingSystem::processRemovedNeighbors(const Engine::ECS::Entity ply, Connection& conn, ECSNetworkingComponent& ecsNetComp) {
		for (auto& [ent, data] : ecsNetComp.neighbors) {
			ENGINE_DEBUG_ASSERT(ent != ply, "A player is not their own neighbor");
			if (data.state != ECSNetworkingComponent::NeighborState::Removed) { continue; }
			if (auto msg = conn.beginMessage<MessageType::ECS_ENT_DESTROY>()) {
				msg.write(ent);
			} else {
				data.state = ECSNetworkingComponent::NeighborState::None;
				ENGINE_WARN("Unable to network entity destroy.");
			}
		}
	}
	
	void EntityNetworkingSystem::processCurrentNeighbors(const Engine::ECS::Entity ply, Connection& conn, ECSNetworkingComponent& ecsNetComp) {
		for (auto& [ent, data] : ecsNetComp.neighbors) {
			ENGINE_DEBUG_ASSERT(ent != ply, "A player is not their own neighbor");
			if (data.state != ECSNetworkingComponent::NeighborState::Added
				&& data.state != ECSNetworkingComponent::NeighborState::Current) {
				continue;
			}

			const auto compsCurr = world.getComponentsBitset(ent);

			// Non-flag components
			Engine::Meta::ForEachIn<CompsSet>::call([&]<class C>{
				constexpr auto cid = world.getComponentId<C>();
				if constexpr (IsNetworkedComponent<C>) {
					if (!world.hasComponent<C>(ent)) { return; }
					const auto& comp = world.getComponent<C>(ent);

					const auto repl = NetworkTraits<C>::getReplType(comp);
					if (repl == Engine::Net::Replication::NONE) { return; }

					const int32 diff = data.comps.test(cid) - world.getComponentsBitset(ent).test(cid);

					if (diff < 0) { // Component Added
						if (networkComponent<C>(ent, conn)) {
							data.comps.set(cid);
						} else {
							ENGINE_WARN("Unable network component add. UPDATE");
						}
					} else if (diff > 0) { // Component Removed
						// TODO: comp removed
					} else if (repl == Engine::Net::Replication::ALWAYS) {
						if (auto msg = conn.beginMessage<MessageType::ECS_COMP_ALWAYS>()) {
							msg.write(ent);
							msg.write(cid);
							if (Engine::ECS::IsSnapshotRelevant<C>) {
								msg.write(world.getTick());
							}

							NetworkTraits<C>::write(comp, msg.getBufferWriter());
						}
					} else if (repl == Engine::Net::Replication::UPDATE) {
						ENGINE_DEBUG_ASSERT("TODO: Update replication is not yet implemented");
						// TODO: impl
					}
				}
			});

			// Flag components
			{
				World::FlagsBitset diffs = data.comps ^ world.getComponentsBitset(ent);
				if (diffs) {
					if (auto msg = conn.beginMessage<MessageType::ECS_FLAG>()) {
						msg.write(ent);
						msg.write(diffs);
						data.comps ^= diffs;
					}
				}
			}
		}
	}

	void EntityNetworkingSystem::updateNeighbors() {
		for (const auto ply : world.getFilter<PlayerFilter>()) {
			auto& ecsNetComp = world.getComponent<ECSNetworkingComponent>(ply);
			const auto& physComp = world.getComponent<PhysicsBodyComponent>(ply);

			ecsNetComp.neighbors.eraseRemove([](auto& pair){
				if (pair.second.state == ECSNetworkingComponent::NeighborState::Removed) {
					return true;
				}

				pair.second.state = ECSNetworkingComponent::NeighborState::Removed;
				return false;
			});

			struct QueryCallbackLarge : b2QueryCallback {
				World& world;
				ECSNetworkingComponent& ecsNetComp;

				QueryCallbackLarge(World& world, ECSNetworkingComponent& ecsNetComp)
					: world{world}, ecsNetComp{ecsNetComp} {
				}

				virtual bool ReportFixture(b2Fixture* fixture) override {
					const Engine::ECS::Entity ent = Game::PhysicsSystem::toEntity(fixture->GetBody()->GetUserData());
					if (ecsNetComp.neighbors.contains(ent)) {
						ecsNetComp.neighbors.get(ent).state = ECSNetworkingComponent::NeighborState::Current;
					}
					return true;
				}
			} queryCallbackLarge(world, ecsNetComp);

			
			struct QueryCallbackSmall : b2QueryCallback {
				const Engine::ECS::Entity ply;
				World& world;
				ECSNetworkingComponent& ecsNetComp;

				QueryCallbackSmall(Engine::ECS::Entity ply, World& world, ECSNetworkingComponent& ecsNetComp)
					: ply{ply}, world{world}, ecsNetComp{ecsNetComp} {
				}

				virtual bool ReportFixture(b2Fixture* fixture) override {
					const Engine::ECS::Entity ent = Game::PhysicsSystem::toEntity(fixture->GetBody()->GetUserData());
					if (!world.hasComponent<NetworkedFlag>(ent)) { return true; }
					if (!ecsNetComp.neighbors.contains(ent) && ent != ply) {
						ecsNetComp.neighbors.add(ent, ECSNetworkingComponent::NeighborState::Added);
					}
					return true;
				}
			} queryCallbackSmall(ply, world, ecsNetComp);


			// We keep objects loaded in a larger area than we initially load them so that
			// if an object is near the edge it doesnt get constantly created and destroyed
			// as a player moves a small amount
			const auto& pos = physComp.getPosition();
			constexpr float32 rangeSmall = 5; // TODO: what range?
			constexpr float32 rangeLarge = 20; // TODO: what range?

			physComp.getWorld()->QueryAABB(&queryCallbackLarge, b2AABB{
				{pos.x - rangeLarge, pos.y - rangeLarge},
				{pos.x + rangeLarge, pos.y + rangeLarge},
			});

			physComp.getWorld()->QueryAABB(&queryCallbackSmall, b2AABB{
				{pos.x - rangeSmall, pos.y - rangeSmall},
				{pos.x + rangeSmall, pos.y + rangeSmall},
			});
		}
	}
}
