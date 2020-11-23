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
	class PhysicsBodyComponent {
		private:
			friend class PhysicsSystem;

			b2Body* body = nullptr;
			int* count = nullptr;

		public:
			PhysicsBodyComponent() = default;
			~PhysicsBodyComponent() noexcept;
			PhysicsBodyComponent(const PhysicsBodyComponent& other) noexcept;
			PhysicsBodyComponent(PhysicsBodyComponent&& other) noexcept;
			void operator=(const PhysicsBodyComponent& other) noexcept;
			void operator=(PhysicsBodyComponent&& other) noexcept;

			void setBody(b2Body* body); // TODO: add constructor arguments world.addComponent

			// TODO: we should get rid of these. Should write wrappers for any funcs we want.
			b2Body& getBody();
			b2World* getWorld();

			// TODO: rename - rm 2
			ENGINE_INLINE void setTransform2(const b2Vec2& pos, float32 ang) { body->SetTransform(pos, ang); }
			ENGINE_INLINE void setTransform2(const b2Transform& trans) { setTransform2(trans.p, trans.q.GetAngle()); };
			
			ENGINE_INLINE const b2Vec2& getPosition() const { return body->GetPosition(); };
			ENGINE_INLINE const auto& getTransform() const { return body->GetTransform(); }
			ENGINE_INLINE const auto& getVelocity() const { return body->GetLinearVelocity(); }
	};
}
