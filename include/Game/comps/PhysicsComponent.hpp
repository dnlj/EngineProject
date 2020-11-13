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
	class PhysicsComponent {
		public:
			constexpr static bool isSnapshotRelevant = true;
			bool snap = false;

		private:
			friend class PhysicsSystem;
			
			// TODO: split into dynamic and static comps?
			// TODO: split interp stuff into own comp.

			struct PhysData {
				b2Transform trans = {};
				b2Vec2 vel = {};
				float32 angVel = {};

				ENGINE_INLINE void to(b2Body* body) const {
					body->SetTransform(trans.p, trans.q.GetAngle());
					body->SetLinearVelocity(vel);
					body->SetAngularVelocity(angVel);
				}

				ENGINE_INLINE void from(b2Body* body) {
					trans = body->GetTransform();
					vel = body->GetLinearVelocity();
					angVel = body->GetAngularVelocity();
				}
			};

			PhysData stored = {};

			b2Body* body = nullptr;
			int* count = nullptr;

			void loadBody() {
				// TODO: better way to set this?
				// TODO: check before set to avoid waking
				stored.to(body);
			}

			void storeBody() {
				stored.from(body);
			}

		public:
			PhysicsComponent() = default;
			~PhysicsComponent() noexcept;
			PhysicsComponent(const PhysicsComponent& other) noexcept;
			PhysicsComponent(PhysicsComponent&& other) noexcept;
			void operator=(const PhysicsComponent& other) noexcept;
			void operator=(PhysicsComponent&& other) noexcept;

			void setBody(b2Body* body); // TODO: add constructor arguments world.addComponent

			// TODO: we should get rid of these. Should write wrappers for any funcs we want.
			b2Body& getBody();
			b2World* getWorld();

			// TODO: rename - rm 2
			ENGINE_INLINE void setTransform2(const b2Transform& trans);
			ENGINE_INLINE void setTransform2(const b2Vec2& pos, float32 ang);
			
			ENGINE_INLINE const b2Vec2& getPosition() const { return body->GetPosition(); };
			ENGINE_INLINE const auto& getTransform() const { return body->GetTransform(); }
			ENGINE_INLINE const auto& getVelocity() const { return body->GetLinearVelocity(); }
			ENGINE_INLINE auto& getStored() { return stored; }
			ENGINE_INLINE const auto& getStored() const { return stored; }


			Engine::Net::Replication netRepl() const;

			void netTo(Connection& conn) const;
			void netToInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Connection& conn) const;
			void netFrom(Connection& conn);
			void netFromInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Connection& conn);
	};
}
