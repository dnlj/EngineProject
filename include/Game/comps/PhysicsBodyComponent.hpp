#pragma once

// Box2D
#include <Box2D/Box2D.h>

// Game
#include <Game/common.hpp>
#include <Game/NetworkTraits.hpp>
#include <Game/systems/PhysicsSystem.hpp>
#include <Game/Zone.hpp>


namespace Game {
	class ZoneInfo {
		public:
			ZoneId id = zoneInvalidId;

			// TODO: should probably be a temp DS on the manager.
			ZoneId group = zoneInvalidId;
	};

	class PhysicsBody {
		private:
			b2Body* body = nullptr;

			// TODO: need to handle networking
		public:
			ZoneInfo zone; // TODO: make private once we have everyhing moved over.

		public:
			ENGINE_INLINE bool valid() const noexcept { return body; }
			void clear() noexcept;

			void setBody(b2Body* body, ZoneId zoneId);
			void setZone(ZoneId zoneId);
			ENGINE_INLINE ZoneId getZoneId() const noexcept { return zone.id; }

			ENGINE_INLINE b2Body* takeOwnership() noexcept {
				const auto temp = body;
				*this = {};
				return temp;
			}
			
			void moveZone(WorldAbsVec oldZoneOffset, ZoneId newZoneId, WorldAbsVec newZoneOffset);

			ENGINE_INLINE const b2Fixture* getFixtureList() const noexcept { return body->GetFixtureList(); }

			ENGINE_INLINE const auto& getTransform() const noexcept { return body->GetTransform(); }
			ENGINE_INLINE void setTransform(const b2Vec2& pos, const float32 ang) { body->SetTransform(pos, ang); }

			ENGINE_INLINE auto getPosition() const noexcept { return body->GetPosition(); };
			ENGINE_INLINE void setPosition(const b2Vec2 p) noexcept { setTransform(p, getAngle()); };

			ENGINE_INLINE float32 getAngle() const noexcept { return body->GetAngle(); };
			ENGINE_INLINE void setAngle(const float32 a) noexcept { setTransform(getPosition(), a); };

			ENGINE_INLINE auto getVelocity() const noexcept { return body->GetLinearVelocity(); };
			ENGINE_INLINE void setVelocity(const b2Vec2 v) noexcept { body->SetLinearVelocity(v); };

			ENGINE_INLINE auto getAngularVelocity() const noexcept { return body->GetAngularVelocity(); };
			ENGINE_INLINE void setAngularVelocity(const float32 av) noexcept { body->SetAngularVelocity(av); };

			ENGINE_INLINE void applyLinearImpulse(const b2Vec2 imp, const bool wake) noexcept { body->ApplyLinearImpulseToCenter(imp, wake); }

			ENGINE_INLINE void setActive(bool active) { body->SetActive(active); }
			ENGINE_INLINE bool getActive(bool active) const noexcept { body->IsActive(); }

			ENGINE_INLINE b2BodyType getType() const noexcept { return body->GetType(); }

			ENGINE_INLINE void createFixture(b2FixtureDef def) {
				ENGINE_DEBUG_ASSERT(body != nullptr, "Attempting to create fixture for null body.");
				ENGINE_DEBUG_ASSERT(zone.id != zoneInvalidId, "Attempting to create fixture without a valid zone.");
				def.filter.groupIndex = zone.id;
				body->CreateFixture(&def);
			}
	};

	// TODO: rename - to just phys comp
	class PhysicsBodyComponent : public PhysicsBody, public MoveOnly {
		public:
			bool snap = false; // TODO: this should probably be on the interp component?
			bool rollbackOverride = false; // TODO: there is probably a better way to handle this.

		public:
			PhysicsBodyComponent() = default;
			void moveZone(WorldAbsVec oldZoneOffset, ZoneId newZoneId, WorldAbsVec newZoneOffset);
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
				bool rollbackOverride = false; // TODO: there is probably a better way to handle this.
			};

			using Container = SparseSet<Entity, Type>;

			static std::tuple<Type> toSnapshot(const Game::PhysicsBodyComponent& obj) noexcept {
				return Type{
					.trans = obj.getTransform(),
					.vel = obj.getVelocity(),
					.angVel = obj.getAngularVelocity(),
					.rollbackOverride = obj.rollbackOverride,
				};
			}

			static void fromSnapshot(Game::PhysicsBodyComponent& obj, const Type& snap) noexcept {
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

			static void writeInit(const PhysicsBodyComponent& obj, Engine::Net::BufferWriter& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent);
			static void write(const PhysicsBodyComponent& obj, Engine::Net::BufferWriter& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent);

			static std::tuple<PhysicsBodyComponent> readInit(Engine::Net::BufferReader& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent);
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
