#pragma once

// Box2D
#include <Box2D/Box2D.h>

// Engine
#include <Engine/Net/Connection.hpp>
#include <Engine/Net/Replication.hpp>

// Game
#include <Game/Common.hpp>
#include <Game/systems/PhysicsSystem.hpp>
#include <Game/Connection.hpp>


namespace Game {
	enum PhysicsType : uint8 {
		Default,
		Player,
	};

	// TODO: rename - to just phys comp
	class PhysicsBodyComponent {
		private:
			friend class PhysicsSystem;
			friend class NetworkingSystem;

			// Hello! Did you modify/add/remove a member variable of this class? Make sure to update the move/copy/assignment functions.
			b2Body* body = nullptr;
			int* count = nullptr; // TODO: is this actually used? since we dont do rollback for this?

		public:
			PhysicsType type = {}; // TODO: why is this public?

			bool snap = false; // TODO: this should probably be on the interp component?
			bool rollbackOverride = false; // TODO: there is probably a better way to handle this.

			struct SnapshotData {
				b2Transform trans = {};
				b2Vec2 vel = {};
				float32 angVel = {};
				bool rollbackOverride = false; // TODO: there is probably a better way to handle this.

				
				void netFrom(Connection& conn) { // TODO: not sure how to handle this and keep in sync with phys comp. There is probably a better solution
					trans = *conn.read<b2Transform>();
					vel = *conn.read<b2Vec2>();
					rollbackOverride = true;
				}
			};

			operator SnapshotData() const noexcept {
				return {
					.trans = body->GetTransform(),
					.vel = body->GetLinearVelocity(),
					.angVel = body->GetAngularVelocity(),
					.rollbackOverride = rollbackOverride,
				};
			}

		public:
			PhysicsBodyComponent() = default;
			~PhysicsBodyComponent() noexcept;
			PhysicsBodyComponent(const PhysicsBodyComponent& other) noexcept;

			PhysicsBodyComponent& operator=(const SnapshotData& other) noexcept {
				setTransform(other.trans.p, other.trans.q.GetAngle());
				setVelocity(other.vel);
				setAngularVelocity(other.angVel);
				rollbackOverride = other.rollbackOverride;
				return *this;
			}

			PhysicsBodyComponent(PhysicsBodyComponent&& other) noexcept;
			void operator=(const PhysicsBodyComponent& other) noexcept;// TODO: should return ref
			void operator=(PhysicsBodyComponent&& other) noexcept;// TODO: should return ref


			void setBody(b2Body* body); // TODO: add constructor arguments world.addComponent

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

			Engine::Net::Replication netRepl() const {
				return (body->GetType() == b2_staticBody) ? Engine::Net::Replication::NONE : Engine::Net::Replication::ALWAYS;
			}

			void netTo(Engine::Net::BufferWriter& buff) const;

			void netToInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Engine::Net::BufferWriter& buff) const;

			void netFrom(Connection& conn);

			void netFromInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Connection& conn);
	};
}
