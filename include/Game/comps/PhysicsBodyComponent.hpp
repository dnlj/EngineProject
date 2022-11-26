#pragma once

// Box2D
#include <Box2D/Box2D.h>

// Game
#include <Game/Common.hpp>
#include <Game/systems/PhysicsSystem.hpp>
#include <Game/NetworkTraits.hpp>


namespace Game {
	enum PhysicsType : uint8 { // TODO: should probably rename filter/collision type or similar
		Default,
		Player,
	};

	// TODO: rename - to just phys comp
	class PhysicsBodyComponent {
		private:
			// Hello! Did you modify/add/remove a member variable of this class? Make sure to update the move/copy/assignment functions.
			b2Body* body = nullptr;

		public:
			PhysicsType type = {}; // TODO: why is this public?
			bool snap = false; // TODO: this should probably be on the interp component?
			bool rollbackOverride = false; // TODO: there is probably a better way to handle this.

		public:
			PhysicsBodyComponent() = default;
			void setBody(b2Body* body);

			// TODO: why does one return pointer and the other ref. Make both ref or pointer.
			// TODO: we should get rid of these. Should write wrappers for any funcs we want.
			b2Body& getBody() { return *body; }
			const b2Body& getBody() const { return *body; }

			b2World* getWorld() { return body->GetWorld(); }
			const b2World* getWorld() const { return body->GetWorld(); }

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

			void setType(PhysicsType t) {
				b2Filter filter;
				filter.groupIndex = t;

				auto fix = body->GetFixtureList();
				while (fix) {
					fix->SetFilterData(filter);
					fix = fix->GetNext();
				}
			}
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
					.trans = obj.getBody().GetTransform(),
					.vel = obj.getBody().GetLinearVelocity(),
					.angVel = obj.getBody().GetAngularVelocity(),
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
				return (obj.getBody().GetType() == b2_staticBody) ? Engine::Net::Replication::ONCE : Engine::Net::Replication::ALWAYS;
			}

			static void writeInit(const PhysicsBodyComponent& obj, Engine::Net::BufferWriter& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent);
			static void write(const PhysicsBodyComponent& obj, Engine::Net::BufferWriter& buff);

			static std::tuple<PhysicsBodyComponent> readInit(Engine::Net::BufferReader& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent);
			static void read(PhysicsBodyComponent& obj, Engine::Net::BufferReader& buff);

			static void read(Engine::ECS::SnapshotTraits<PhysicsBodyComponent>::Type& obj, Engine::Net::BufferReader& buff) {
				buff.read<b2Transform>(&obj.trans);
				buff.read<b2Vec2>(&obj.vel);
				obj.rollbackOverride = true;
			}
	};
}
