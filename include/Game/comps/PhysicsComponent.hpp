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

		private:
			friend class PhysicsSystem;
			
			// TODO: split into dynamic and static comps.
			// TODO: split interp stuff into own comp.

			b2Transform storedTransform;
			b2Vec2 storedVelocity;
			float32 storedAngularVelocity;

			b2Transform prevTransform;
			b2Transform interpTransform;
			b2Transform remoteTransform;
			b2Body* body = nullptr;
			int* count = nullptr;

			void loadBody() {
				// TODO: better way to set this?
				// TODO: check before set to avoid waking
				body->SetTransform(storedTransform.p, storedTransform.q.GetAngle());
				body->SetLinearVelocity(storedVelocity);
				body->SetAngularVelocity(storedAngularVelocity);
			}

			void storeBody() {
				storedTransform = body->GetTransform();
				storedVelocity = body->GetLinearVelocity();
				storedAngularVelocity = body->GetAngularVelocity();
			}

		public:
			PhysicsComponent() = default;
			~PhysicsComponent();
			PhysicsComponent(const PhysicsComponent& other);
			PhysicsComponent(PhysicsComponent&& other);
			void operator=(const PhysicsComponent& other);
			void operator=(PhysicsComponent&& other);

			void setBody(b2Body* body); // TODO: add constructor arguments world.addComponent

			// TODO: we should get rid of these. Should write wrappers for any funcs we want.
			b2Body& getBody();
			b2World* getWorld();

			void updateTransform(const b2Transform& trans);
			void updateTransform(const b2Vec2& pos, float32 ang);
			void setTransform(const b2Transform& trans);
			void setTransform(const b2Vec2& pos, float32 ang);
			const b2Vec2& getPosition() const;
			const b2Vec2& getInterpPosition() const;

			Engine::Net::Replication netRepl() const;
			void netTo(Connection& conn) const;
			void netToInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Connection& conn) const;
			void netFrom(Connection& conn);
			void netFromInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Connection& conn);
	};
}
