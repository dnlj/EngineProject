// STD
#include <chrono>

// Engine
#include <Engine/Meta/for.hpp>

// Game
#include <Game/World.hpp>
#include <Game/systems/EntityNetworkingSystem.hpp>
#include <Game/comps/all.hpp>

#include <Game/systems/NetworkingSystem.hpp>
#include <Game/Connection.hpp>


////////////////////////////////////////////////////////////////////////////////
// Shared
////////////////////////////////////////////////////////////////////////////////
namespace {
	using namespace Game;
	using PlayerFilter = Engine::ECS::EntityFilterList<
		PlayerFlag,
		ConnectedFlag
	>;

	using Engine::ECS::Entity;
	using Engine::Net::MessageHeader;
	using Engine::Net::BufferReader;
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
		ENGINE_LOG("ECS_INIT - Remote: ", remote, " Local: ", from.ent, " Tick: ", world.getTick(), " - ", tick);

		// TODO: use ping, loss, etc to pick good offset value. We dont actually have good quality values for those stats yet at this point.
		world.setNextTick(tick + 16);
		auto& ensSystem = world.getSystem<Game::EntityNetworkingSystem>();
		auto& entToLocal = ensSystem.getRemoteToLocalEntityMapping();

		ENGINE_DEBUG_ASSERT(entToLocal.size() == 0, "Networked entity map already has entries. This is a bug.");
		entToLocal[remote] = from.ent;

		// TODO: move addPlayer to EntityNetworkingSystem
		world.getSystem<Game::NetworkingSystem>().addPlayer(from);
	}

	void recv_ECS_ENT_CREATE(EngineInstance& engine, ConnectionInfo& from, const MessageHeader head, BufferReader& msg) {
		Entity remote;
		if (!msg.read(&remote)) { return; }

		auto& world = engine.getWorld();
		auto& ensSystem = world.getSystem<Game::EntityNetworkingSystem>();
		auto& entToLocal = ensSystem.getRemoteToLocalEntityMapping();

		auto& local = entToLocal[remote];
		if (local == Engine::ECS::INVALID_ENTITY) {
			local = world.createEntity();
		}

		world.addComponent<NetworkedFlag>(local);
		ENGINE_LOG("Networked: ", local, world.hasComponent<NetworkedFlag>(local));

		ENGINE_LOG("ECS_ENT_CREATE - Remote: ", remote, " Local: ", local, " Tick: ", world.getTick());

		// TODO: components init
	}

	void recv_ECS_ENT_DESTROY(EngineInstance& engine, ConnectionInfo& from, const MessageHeader head, BufferReader& msg) {
		Engine::ECS::Entity remote;
		if (!msg.read(&remote)) { return; }
		
		auto& world = engine.getWorld();
		auto& ensSystem = world.getSystem<Game::EntityNetworkingSystem>();
		auto& entToLocal = ensSystem.getRemoteToLocalEntityMapping();

		auto found = entToLocal.find(remote);
		if (found != entToLocal.end()) {
			world.deferedDestroyEntity(found->second);
			entToLocal.erase(found);
		}
	}

	void recv_ECS_COMP_ADD(EngineInstance& engine, ConnectionInfo& from, const MessageHeader head, BufferReader& msg) {
		Engine::ECS::Entity remote;
		if (!msg.read(&remote)) { return; }

		Engine::ECS::ComponentId cid;
		if (!msg.read(&cid)) { return; }
		
		auto& world = engine.getWorld();
		auto& ensSystem = world.getSystem<Game::EntityNetworkingSystem>();
		auto& entToLocal = ensSystem.getRemoteToLocalEntityMapping();

		auto found = entToLocal.find(remote);
		if (found == entToLocal.end()) { return; }
		auto local = found->second;

		world.callWithComponent(cid, [&]<class C>{
			if constexpr (IsNetworkedComponent<C>) {
				if (!world.hasComponent<C>(local)) {
					ENGINE_FLATTEN std::apply([&]<class... Args>(Args&&... args) ENGINE_INLINE {
						world.addComponent<C>(local, std::forward<Args>(args)...);
					}, NetworkTraits<C>::readInit(msg, engine, world, local));
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
		auto& ensSystem = world.getSystem<Game::EntityNetworkingSystem>();
		auto& entToLocal = ensSystem.getRemoteToLocalEntityMapping();

		auto found = entToLocal.find(remote);
		if (found == entToLocal.end()) { return; }
		auto local = found->second;
		 
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
					NetworkTraits<C>::read(world.getComponent<C>(local), msg);
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
		auto& ensSystem = world.getSystem<Game::EntityNetworkingSystem>();
		auto& entToLocal = ensSystem.getRemoteToLocalEntityMapping();


		auto found = entToLocal.find(remote);
		if (found == entToLocal.end()) { return; }
		auto local = found->second;

		#if ENGINE_DEBUG
			ENGINE_DEBUG_ASSERT(ensSystem._debug_networking == false);
			ensSystem._debug_networking = true;
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
			ensSystem._debug_networking = false;
		#endif
	}

	void recv_PLAYER_DATA(EngineInstance& engine, ConnectionInfo& from, const MessageHeader head, BufferReader& msg) {
		Engine::ECS::Tick tick;
		b2Transform trans;
		b2Vec2 vel;
		
		// TODO: angVel

		if (!msg.read(&tick) || !msg.read(&trans) || !msg.read(&vel)) {
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
		if (diff.LengthSquared() >  eps * eps) { // TODO: also check q
			ENGINE_INFO(std::setprecision(std::numeric_limits<decltype(physCompState.trans.p.x)>::max_digits10),
				"Oh boy a mishap has occured on tick ", tick,
				" (<", physCompState.trans.p.x, ", ", physCompState.trans.p.y, "> - <",
				trans.p.x, ", ", trans.p.y, "> = <",
				diff.x, ", ", diff.y,
				">)"
			);

			physCompState.trans = trans;
			physCompState.vel = vel;
			physCompState.rollbackOverride = true;

			world.scheduleRollback(tick);
		}

		// TODO: vel
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
	}
}

#endif // ENGINE_CLIENT

////////////////////////////////////////////////////////////////////////////////
// Server side
////////////////////////////////////////////////////////////////////////////////
#if ENGINE_SERVER
namespace Game {
	void EntityNetworkingSystem::update(float32 dt) {
		static_assert(ENGINE_SERVER, "This code is server side only.");
		const auto now = world.getTime();
		if (now < nextUpdate) { return; }

		// TODO: config for this
		nextUpdate = now + std::chrono::milliseconds{1000 / 20};

		updateNeighbors();

		auto& netSys = world.getSystem<NetworkingSystem>();
		for (auto& ply : world.getFilter<PlayerFilter>()) {
			auto& ecsNetComp = world.getComponent<ECSNetworkingComponent>(ply);

			auto* conn = netSys.getConnection(ply);
			if (!conn) {
				ENGINE_WARN("Unable to get connection for entity. This is a bug.");
				continue;
			}

			// TODO: see DqVBIIfY
			// TODO: move elsewhere, this isnt really related to ECS networking
			{ // TODO: player data should be sent every tick along with actions/inputs.
			// TODO: cont.  Should it? every few frames is probably fine for keeping it in sync. Although when it does desync it will be a larger rollback.
				const auto& physComp = world.getComponent<PhysicsBodyComponent>(ply);
				if (auto msg = conn->beginMessage<MessageType::PLAYER_DATA>()) {
					msg.write(world.getTick() + 1); // since this is in `run` and not before `tick` we are sending one tick off. +1 is temp fix
					msg.write(physComp.getTransform());
					msg.write(physComp.getVelocity());
				}
			}

			// TODO: handle entities without phys comp?
			// TODO: figure out which entities have been updated
			// TODO: prioritize entities

			// Order is important here since some failed writes change neighbor states
			processAddedNeighbors(ply, *conn, ecsNetComp);
			processRemovedNeighbors(ply, *conn, ecsNetComp);
			processCurrentNeighbors(ply, *conn, ecsNetComp);
		}
	}

	template<class C>
	bool EntityNetworkingSystem::networkComponent(const Entity ent, Connection& conn) const {
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
					const Entity ent = Game::PhysicsSystem::toEntity(fixture->GetBody()->GetUserData());
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
					const Entity ent = Game::PhysicsSystem::toEntity(fixture->GetBody()->GetUserData());
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
#endif // ENGINE_SERVER
