// STD
#include <chrono>

// Engine
#include <Engine/Meta/for.hpp>

// Game
#include <Game/Connection.hpp>
#include <Game/World.hpp>
#include <Game/comps/all.hpp>
#include <Game/systems/EntityNetworkingSystem.hpp>
#include <Game/systems/NetworkingSystem.hpp>
#include <Game/systems/ZoneManagementSystem.hpp>

// TODO: rm - shouldnt be accessed from here.
#include <Game/systems/MapSystem.hpp>


////////////////////////////////////////////////////////////////////////////////
// Shared
////////////////////////////////////////////////////////////////////////////////
namespace {
	using namespace Game;
	using PlayerFilter = Engine::ECS::EntityFilterList<PlayerFlag>;

	using Engine::ECS::Entity;
	using Engine::Net::MessageHeader;
	using Engine::Net::BufferReader;

	using NeighborState = ECSNetworkingComponent::NeighborState;
}

////////////////////////////////////////////////////////////////////////////////
// Client side
////////////////////////////////////////////////////////////////////////////////
#if ENGINE_CLIENT
namespace {
	void recv_ECS_INIT(EngineInstance& engine, ConnectionInfo& from, const MessageHeader head, BufferReader& msg) {
		Engine::ECS::Entity remote;
		if (!msg.read(&remote)) {
			ENGINE_WARN("Server didn't send remote entity. Unable to sync.");
			return;
		}

		Engine::ECS::Tick tick;
		if (!msg.read(&tick)) {
			ENGINE_WARN("Unable to sync ticks.");
			return;
		}

		auto& world = engine.getWorld();
		ENGINE_LOG("ECS_INIT - Remote: ", remote, " Local: ", from.ent, " OldTick: ", world.getTick(), " NewTick: ", tick);

		// It is important that we add the player _AFTER_ setting the next tick
		// or else we can run into issues where initial entity/component state
		// is incorrect because it was initialized with the old tick value. The
		// ActionComponent action sequence buffer is one example.
		// TODO: use ping, loss, etc to pick good offset value. We dont actually have good quality values for those stats yet at this point.
		world.setNextTick(tick + 16);
		world.getSystem<Game::NetworkingSystem>().addPlayer(from);

		auto& entNetSystem = world.getSystem<Game::EntityNetworkingSystem>();
		ENGINE_DEBUG_ASSERT(entNetSystem.getEntityMapping().size() == 0, "Networked entity map already has entries. This is a bug.");
		ENGINE_DEBUG_ASSERT(remote, "Attempting to setup invalid entity mapping (remote)");
		ENGINE_DEBUG_ASSERT(from.ent, "Attempting to setup invalid entity mapping (local)");
		entNetSystem.addEntityMapping(remote, from.ent);
	}

	void recv_ECS_ENT_CREATE(EngineInstance& engine, ConnectionInfo& from, const MessageHeader head, BufferReader& msg) {
		Entity remote;
		if (!msg.read(&remote)) { return; }

		auto& world = engine.getWorld();
		auto& entNetSystem = world.getSystem<Game::EntityNetworkingSystem>();

		if (entNetSystem.getEntityMapping(remote)) {
			ENGINE_WARN("Attempting to create duplicate entity (remote = ", remote, ")");
			return;
		}

		auto local = world.createEntity();
		entNetSystem.addEntityMapping(remote, local);
		world.addComponent<NetworkedFlag>(local);
		
		ENGINE_DEBUG_ASSERT(remote, "Attempting to setup invalid entity mapping (remote)");
		ENGINE_DEBUG_ASSERT(from.ent, "Attempting to setup invalid entity mapping (local)");
		ENGINE_LOG("ECS_ENT_CREATE - Remote: ", remote, " Local: ", local, " Tick: ", world.getTick());

		// TODO: components init
	}

	void recv_ECS_ENT_DESTROY(EngineInstance& engine, ConnectionInfo& from, const MessageHeader head, BufferReader& msg) {
		Engine::ECS::Entity remote;
		if (!msg.read(&remote)) { return; }
		
		auto& world = engine.getWorld();
		auto& entNetSystem = world.getSystem<Game::EntityNetworkingSystem>();

		if (auto local = entNetSystem.removeEntityMapping(remote)) {
			ENGINE_LOG("ECS_ENT_DESTROY - Remote: ", remote, " Local: ", local, " Tick: ", world.getTick());
			world.deferedDestroyEntity(local);
		}
	}

	void recv_ECS_COMP_ADD(EngineInstance& engine, ConnectionInfo& from, const MessageHeader head, BufferReader& msg) {
		Engine::ECS::Entity remote;
		if (!msg.read(&remote)) { return; }

		Engine::ECS::ComponentId cid;
		if (!msg.read(&cid)) { return; }
		
		ENGINE_INFO2("recv_ECS_COMP_ADD: {} {}", remote, cid);

		auto& world = engine.getWorld();
		auto& entNetSystem = world.getSystem<Game::EntityNetworkingSystem>();
		auto local = entNetSystem.getEntityMapping(remote);
		if (!local) {
			ENGINE_WARN("Attempting to add component to uncreated local entity.");
			return;
		}

		world.callWithComponent(cid, [&]<class C>{
			if constexpr (IsNetworkedComponent<C>) {
				if (!world.hasComponent<C>(local)) {
					NetworkTraits<C>::readInit(msg, engine, world, local);
				}
			} else if constexpr (ENGINE_DEBUG) {
				ENGINE_WARN("Attemping to network non-network component");
			}
		});
	}

	void recv_ECS_COMP_ALWAYS(EngineInstance& engine, ConnectionInfo& from, const MessageHeader head, BufferReader& msg) {
		Engine::ECS::Entity remote;
		if (!msg.read(&remote)) { return; }

		Engine::ECS::ComponentId cid;
		if (!msg.read(&cid)) { return; }
		
		auto& world = engine.getWorld();
		auto& entNetSystem = world.getSystem<Game::EntityNetworkingSystem>();
		auto local = entNetSystem.getEntityMapping(remote);

		if (!local) {
			ENGINE_WARN("Attempting to update uncreated local entity.");
			return;
		}
		 
		if (!world.isAlive(local)) {
			ENGINE_WARN("Attempting to update dead entitiy ", local);
			return;
		}

		// TODO: if a component IsSnapshotRelevant we should still store the snapshot data
		if (!world.hasComponent(local, cid)) {
			ENGINE_WARN(local, " does not have component ", cid);
			return;
		}

		world.callWithComponent(cid, [&]<class C>{
			if constexpr (IsNetworkedComponent<C>) {
				// TODO: this is a somewhat strange way to handle this
				if constexpr (Engine::ECS::IsSnapshotRelevant<C>) {
					Engine::ECS::Tick tick;
					if (!msg.read(&tick)) {
						ENGINE_WARN("No tick specified for snapshot component in ECS_COMP_ALWAYS");
						return;
					}

					NetworkTraits<C>::read(world.getComponentState<C>(local, tick), msg);
				} else {
					NetworkTraits<C>::read(world.getComponent<C>(local), msg, engine, world, local);
				}
			} else if constexpr (ENGINE_DEBUG) {
				ENGINE_WARN("Attemping to network non-network component");
			}
		});
	}

	void recv_ECS_FLAG(EngineInstance& engine, ConnectionInfo& from, const MessageHeader head, BufferReader& msg) {
		Engine::ECS::Entity remote;
		if (!msg.read(&remote)) { return; }

		Game::World::FlagsBitset flags;
		if (!msg.read(&flags)) { return; }
		
		auto& world = engine.getWorld();
		auto& entNetSystem = world.getSystem<Game::EntityNetworkingSystem>();
		auto local = entNetSystem.getEntityMapping(remote);

		if (!local) {
			ENGINE_WARN("Attempting to set flags on uncreated local entity.");
			return;
		}

		#if ENGINE_DEBUG
			ENGINE_DEBUG_ASSERT(entNetSystem._debug_networking == false);
			entNetSystem._debug_networking = true;
		#endif

		Engine::Meta::ForEachIn<Game::FlagsSet>::call([&]<class C>{
			constexpr auto cid = world.getComponentId<C>();
			if (!flags.test(cid)) { return; }

			if (world.hasComponent<C>(local)) {
				world.removeComponent<C>(local);
			} else {
				world.addComponent<C>(local);
			}
		});

		#if ENGINE_DEBUG
			entNetSystem._debug_networking = false;
		#endif
	}

	void recv_PLAYER_DATA(EngineInstance& engine, ConnectionInfo& from, const MessageHeader head, BufferReader& msg) {
		Engine::ECS::Tick tick;
		b2Transform trans;
		b2Vec2 vel;
		ZoneId zoneId;
				
		// TODO: angVel

		if (!msg.read(&tick) || !msg.read(&trans) || !msg.read(&vel) || !msg.read(&zoneId)) {
			ENGINE_WARN("Invalid PLAYER_DATA network message");
			return;
		}
		
		auto& world = engine.getWorld();
		if (!world.hasComponent<PhysicsBodyComponent>(from.ent)) {
			ENGINE_WARN("PLAYER_DATA message received for entity that has no PhysicsBodyComponent");
			return;
		}

		auto& physCompState = world.getComponentState<PhysicsBodyComponent>(from.ent, tick);
		const auto diff = physCompState.trans.p - trans.p;
		const float32 eps = 0.0001f; // TODO: figure out good eps value. Probably half the size of a pixel or similar.
		//if (diff.LengthSquared() > 0.0001f) { // TODO: also check q
		// TODO: why does this ever happen with only one player connected?
		if (diff.LengthSquared() > eps * eps) { // TODO: also check q
			ENGINE_INFO2(
				"Oh boy a mishap has occured on tick {} with: {} - {} = {}",
				tick, physCompState.trans.p, trans.p, diff
			),

			physCompState.trans = trans;
			physCompState.vel = vel;
			physCompState.rollbackOverride = true;
			physCompState.zoneId = zoneId;

			// Don't rollback between zones.
			const auto& physComp = world.getComponent<PhysicsBodyComponent>(from.ent);
			if (zoneId == physComp.getZoneId()) {
				world.scheduleRollback(tick);
			}
		}

		// TODO: vel
	}

	void recv_ECS_ZONE_INFO(EngineInstance& engine, ConnectionInfo& from, const MessageHeader head, BufferReader& msg) {
		ZoneId zoneId;
		WorldAbsVec zonePos;

		if (!msg.read(&zoneId) || !msg.read(&zonePos)) {
			ENGINE_WARN2("Unable to read zone info in ECS_ZONE_INFO.");
			ENGINE_DEBUG_BREAK;
			return;
		}
		
		auto& world = engine.getWorld();
		auto& entNetSys = world.getSystem<EntityNetworkingSystem>();
		auto& zoneSys = world.getSystem<ZoneManagementSystem>();

		// We should only be getting this message when the player switches zones
		// so it is safe to assume that the player also migrated zones.
		zoneSys.ensureZone(zoneId, zonePos);
		zoneSys.migratePlayer(from.ent, zoneId, world.getComponent<PhysicsBodyComponent>(from.ent));

		// TODO: Find a better solution, this is just a hack for now. The issues
		//       is terrain zone isn't updated until the next tick whereas this is
		//       received on update so there are a few frames where entities have
		//       moved but terrain is still in the old zone.
		world.getSystem<MapSystem>().ensurePlayAreaLoaded(from.ent);

		while (msg.remaining()) {
			Entity remote;
			if (!msg.read(&remote)) {
				ENGINE_WARN2("Unable to read entity in ECS_ZONE_INFO.");
				ENGINE_DEBUG_BREAK;
				return;
			}

			const auto local = entNetSys.getEntityMapping(remote);
			zoneSys.migrateEntity(local, zoneId, world.getComponent<PhysicsBodyComponent>(local));
		}
	}
}

namespace Game {
	void EntityNetworkingSystem::setup() {
		static_assert(ENGINE_CLIENT, "This code is client side only");
		auto& netSys = world.getSystem<NetworkingSystem>();
		netSys.setMessageHandler(MessageType::ECS_INIT, recv_ECS_INIT);
		netSys.setMessageHandler(MessageType::ECS_ENT_CREATE, recv_ECS_ENT_CREATE);
		netSys.setMessageHandler(MessageType::ECS_ENT_DESTROY, recv_ECS_ENT_DESTROY);
		netSys.setMessageHandler(MessageType::ECS_COMP_ADD, recv_ECS_COMP_ADD);
		netSys.setMessageHandler(MessageType::ECS_COMP_ALWAYS, recv_ECS_COMP_ALWAYS);
		netSys.setMessageHandler(MessageType::ECS_FLAG, recv_ECS_FLAG);
		netSys.setMessageHandler(MessageType::PLAYER_DATA, recv_PLAYER_DATA);
		netSys.setMessageHandler(MessageType::ECS_ZONE_INFO, recv_ECS_ZONE_INFO);
	}
}

#endif // ENGINE_CLIENT

////////////////////////////////////////////////////////////////////////////////
// Server side
////////////////////////////////////////////////////////////////////////////////
#if ENGINE_SERVER
namespace Game {
	void EntityNetworkingSystem::tick() {
		// Neighbors for all entities must be updated at the same time to make
		// sure we have consistent zone changes and other logic if an entity is
		// the neighbor of two or more players. We can not move this to
		// EntityNetworkingSystem::network or we will be breaking that.
		//
		// We also must do this every tick or else the neighbors may be
		// inconsistent with the physics if there is a very fast moving entity
		// or an entity is teleported.
		updateNeighbors();
	}
	
	void EntityNetworkingSystem::network(NetPlySet plys) {
		static_assert(ENGINE_SERVER, "This code is server side only.");

		for (auto& ply : world.getFilter<PlayerFilter>()) {
			auto& ecsNetComp = world.getComponent<ECSNetworkingComponent>(ply);
			auto& conn = world.getComponent<NetworkComponent>(ply).get();

			// TODO: see DqVBIIfY
			// TODO: move elsewhere, this isnt really related to ECS networking
			// 
			// TODO: player data should be sent every tick along with actions/inputs.
			//       Should it? every few frames is probably fine for keeping it in
			//       sync. Although when it does desync it will be a larger rollback.
			const auto& physComp = world.getComponent<PhysicsBodyComponent>(ply);
			if (auto msg = conn.beginMessage<MessageType::PLAYER_DATA>()) {
				// TODO: Is this +1 comment still correct? Shouldn't this be
				//       removed/updated since networking now happens at the start of
				//       the loop instead of the end?
				msg.write(world.getTick() + 1); // since this is in `update` and not before `tick` we are sending one tick off. +1 is temp fix
				msg.write(physComp.getTransform());
				msg.write(physComp.getVelocity());
				msg.write(physComp.getZoneId());
			}

			// TODO: prioritize entities?

			// Order is important here since some failed writes change neighbor states
			zoneChanged.clear();
			for (auto& [ent, data] : ecsNetComp.neighbors) {
				if (data.test(NeighborState::AddedAny)) {
					processAddedNeighbor(conn, ent, data);
				} else if (data.test(NeighborState::RemovedAny)) {
					processRemovedNeighbor(conn, ent, data);
				} else if (data.test(NeighborState::ZoneChanged)) {
					zoneChanged.push_back(ent);
					data.reset(NeighborState::Current);
				} else {
					// Is a current neighbor, do nothing. Handled below.
				}

				// Regardless of the current state, as long as the entity wasn't
				// removed, we need to send the current data.
				if (!data.test(NeighborState::Removed) && !data.get() == NeighborState::None) {
					processCurrentNeighbor(conn, ent, data);
				}
			}

			if (!zoneChanged.empty() || ecsNetComp.plyZoneChanged) {
				if (auto msg = conn.beginMessage<MessageType::ECS_ZONE_INFO>()) {
					const auto& zoneSys = world.getSystem<ZoneManagementSystem>();
					const auto zoneId = physComp.getZoneId();
					const auto zonePos = zoneSys.getZone(zoneId).offset;
					msg.write(zoneId);
					msg.write(zonePos);

					for (const auto ent : zoneChanged) {
						msg.write(ent);
						ENGINE_INFO2("ECS_ZONE_INFO (each): {} {} {}", ent, zoneId, zonePos);
					}
				} else {
					// TODO: This code path is largely untested. ATM we don't have a way
					//       to force messages to fail to fully verify this.
					ENGINE_WARN2("Unable to send entity zone change.");
					ENGINE_DEBUG_BREAK;
					for (const auto ent : zoneChanged) {
						ecsNetComp.neighbors[ent].reset(NeighborState::ZoneChanged);
					}
				}
			}

			ecsNetComp.plyZoneChanged = false;

			// Remove any neighbors that were successfully networked.
			ecsNetComp.neighbors.eraseRemove([](const auto& pair){
				return pair.second.get() == NeighborState::None;
			});
		}
	}

	template<class C>
	bool EntityNetworkingSystem::networkComponent(const Entity ent, Connection& conn) const {
		auto& comp = world.getComponent<C>(ent);
		if (NetworkTraits<C>::getReplType(comp) == Engine::Net::Replication::NONE) { return true; }

		if (auto msg = conn.beginMessage<MessageType::ECS_COMP_ADD>()) {
			ENGINE_INFO2("ECS_COMP_ADD: {} {}", ent, world.getComponentId<C>());
			msg.write(ent);
			msg.write(world.getComponentId<C>());
			NetworkTraits<C>::writeInit(comp, msg.getBufferWriter(), engine, world, ent);
			return true;
		}

		return false;
	}

	void EntityNetworkingSystem::processAddedNeighbor(Connection& conn, const Engine::ECS::Entity ent, ECSNetworkingComponent::NeighborData& data) {
		if (auto msg = conn.beginMessage<MessageType::ECS_ENT_CREATE>()) {
			data.reset(NeighborState::Current);
			ENGINE_LOG2("ECS_ENT_CREATE: {}", ent);
			msg.write(ent);
		} else {
			data.reset(NeighborState::None);
			ENGINE_WARN("Unable to network entity create.");
		}
	}

	void EntityNetworkingSystem::processRemovedNeighbor(Connection& conn, const Engine::ECS::Entity ent, ECSNetworkingComponent::NeighborData& data) {
		if (auto msg = conn.beginMessage<MessageType::ECS_ENT_DESTROY>()) {
			data.reset(NeighborState::None);
			ENGINE_LOG2("ECS_ENT_DESTROY: {}", ent);
			msg.write(ent);
		} else {
			ENGINE_WARN("Unable to network entity destroy.");
		}
	}
	
	void EntityNetworkingSystem::processCurrentNeighbor(Connection& conn, const Engine::ECS::Entity ent, ECSNetworkingComponent::NeighborData& data) {
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

						NetworkTraits<C>::write(comp, msg.getBufferWriter(), engine, world, ent);
					}
				} else if (repl == Engine::Net::Replication::UPDATE) {
					ENGINE_DEBUG_ASSERT("TODO: Update replication is not yet implemented");
					// TODO: impl
				}
			}
		});

		// Flag components
		{
			constexpr World::FlagsBitset netFlagMask = Engine::Meta::forAll<FlagsSet>([]<class... Fs>{
				World::FlagsBitset res = {};
				res = (... | (IsNetworkedFlag<Fs> ? (World::FlagsBitset{1} << World::getComponentId<Fs>()) : 0));
				return res;
			});

			World::FlagsBitset diffs = (data.comps ^ world.getComponentsBitset(ent)) & netFlagMask;
			if (diffs) {
				if (auto msg = conn.beginMessage<MessageType::ECS_FLAG>()) {
					msg.write(ent);
					msg.write(diffs);
					data.comps ^= diffs;
				}
			}
		}
	}
	
	void EntityNetworkingSystem::updateNeighbors() {
		auto& physSys = world.getSystem<PhysicsSystem>();
		auto& physWorld = physSys.getPhysicsWorld();

		for (const auto ply : world.getFilter<PlayerFilter>()) {
			auto& ecsNetComp = world.getComponent<ECSNetworkingComponent>(ply);
			const auto& physComp = world.getComponent<PhysicsBodyComponent>(ply);

			// Flag everything as might be removed. Current and added neighbors
			// will get overwritten in the persist and add queries.
			for (auto& [ent, data] : ecsNetComp.neighbors) {
				data.maybeRemoved();
			}

			// Create a b2 query object the filters entities to only those relevant the the player.
			const auto createQuery = []<class Func>(ZoneId plyZoneId, World& world, Func&& func){
				struct ZoneQuery : b2QueryCallback {
					Func func;
					ZoneId plyZoneId = 0;
					World& world;

					ZoneQuery(ZoneId plyZoneId, World& world, Func&& func)
						: func{std::move(func)}, plyZoneId{plyZoneId}, world{world} {
					}

					virtual bool ReportFixture(b2Fixture* fixture) override {
						const Entity ent = Game::PhysicsSystem::toEntity(fixture);
						if (world.hasComponent<NetworkedFlag>(ent)) {
							const auto& entPhysComp = world.getComponent<PhysicsBodyComponent>(ent);
							if (entPhysComp.getZoneId() == plyZoneId) {
								func(*this, ent);
							}
						}
						return true;
					}
				} query{plyZoneId, world, std::move(func)};
				return query;
			};

			// Persist any items that are already neighbors in a larger area to
			// avoid rapid add/remove near edge/border if the player is moving
			// around in the same small area a lot.
			auto queryPersist = createQuery(physComp.getZoneId(), world,
				[&ecsNetComp](const auto& self, Entity ent){
					if (ecsNetComp.neighbors.contains(ent)) {
						auto& data = ecsNetComp.neighbors.get(ent);
						data.current();
					}
				}
			);

			// Only add new items in a smaller radius.
			auto queryAdd = createQuery(physComp.getZoneId(), world,
				[ply, &ecsNetComp](const auto& self, Entity ent){
					if (!ecsNetComp.neighbors.contains(ent) && ent != ply) {
						ecsNetComp.neighbors.add(ent, NeighborState::Added);
					}
				}
			);

			// TODO: maybe these should be cvars?
			const auto& pos = physComp.getPosition();

			physWorld.QueryAABB(&queryPersist, b2AABB{
				{pos.x - neighborRangePersist, pos.y - neighborRangePersist},
				{pos.x + neighborRangePersist, pos.y + neighborRangePersist},
			});

			physWorld.QueryAABB(&queryAdd, b2AABB{
				{pos.x - neighborRangeAdd, pos.y - neighborRangeAdd},
				{pos.x + neighborRangeAdd, pos.y + neighborRangeAdd},
			});

			// Mark anything was wasn't persisted or added for removal.
			for (auto& [ent, data] : ecsNetComp.neighbors) {
				if (data.test(NeighborState::MaybeRemoved) && !data.test(NeighborState::RemovedAny)) {
					data.removed();
				}
			}
		}
	}
}
#endif // ENGINE_SERVER
