#pragma once

// Box2D
#include <Box2D/Box2D.h>

// Game
#include <Game/common.hpp>
#include <Game/NetworkTraits.hpp>
#include <Game/systems/PhysicsSystem.hpp>
#include <Game/PhysicsBody.hpp>


namespace Game {
	// TODO: rename - to just phys comp
	class PhysicsBodyComponent : public PhysicsBody, public MoveOnly {
		public:
			bool snap = false; // TODO: this should probably be on the interp component?
			bool rollbackOverride = false; // TODO: there is probably a better way to handle this.

		public:
			PhysicsBodyComponent(PhysicsBody body);

			/**
			 * @see PhysicsBody::setZone
			 */
			void setZone(ZoneId zoneId);

			/**
			 * @see PhysicsBody::moveZone
			 */
			void moveZone(WorldAbsVec oldZoneOffset, ZoneId newZoneId, WorldAbsVec newZoneOffset);

		private:
			// Ensure we don't use these directly. Whenever the zone is modified we need to ensure
			// snap is set to avoid interp errors.
			using PhysicsBody::setZone;
			using PhysicsBody::moveZone;
	};
}

namespace Engine::ECS {
	template<>
	class SnapshotTraits<Game::PhysicsBodyComponent> {
		public:
			struct Type {
				b2Transform trans = {};
				b2Vec2 vel = {};
				float32 angVel = {};
				Game::ZoneId zoneId = {};
				bool rollbackOverride = false; // TODO: there is probably a better way to handle this.
			};

			using Container = SparseSet<Entity, Type>;

			static std::tuple<Type> toSnapshot(const Game::PhysicsBodyComponent& obj) noexcept {
				return Type{
					.trans = obj.getTransform(),
					.vel = obj.getVelocity(),
					.angVel = obj.getAngularVelocity(),
					.zoneId = obj.getZoneId(),
					.rollbackOverride = obj.rollbackOverride,
				};
			}

			static void fromSnapshot(Game::PhysicsBodyComponent& obj, const Type& snap) noexcept {
				// Don't rollback between zones
				if (obj.getZoneId() != snap.zoneId) { return; }

				obj.setTransform(snap.trans.p, snap.trans.q.GetAngle());
				obj.setVelocity(snap.vel);
				obj.setAngularVelocity(snap.angVel);
				obj.rollbackOverride = snap.rollbackOverride;
			}
	};
}

namespace Game {
	template<>
	class NetworkTraits<PhysicsBodyComponent> {
		public:
			static Engine::Net::Replication getReplType(const PhysicsBodyComponent& obj) {
				return (obj.getType() == b2_staticBody) ? Engine::Net::Replication::ONCE : Engine::Net::Replication::ALWAYS;
			}

			static void writeInit(const PhysicsBodyComponent& obj, Engine::Net::StaticBufferWriter& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent);
			static void write(const PhysicsBodyComponent& obj, Engine::Net::StaticBufferWriter& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent);

			static void readInit(Engine::Net::BufferReader& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent);
			static void read(PhysicsBodyComponent& obj, Engine::Net::BufferReader& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent);

			static void read(Engine::ECS::SnapshotTraits<PhysicsBodyComponent>::Type& obj, Engine::Net::BufferReader& buff) {
				buff.read<b2Transform>(&obj.trans);
				buff.read<b2Vec2>(&obj.vel);
				obj.rollbackOverride = true;

				// ZoneId isnt relevant to snapshots. Will be handled be the
				// newer reads + zone specific messages.
				ZoneId zoneId_unused;
				buff.read<ZoneId>(&zoneId_unused);
			}
	};
}
