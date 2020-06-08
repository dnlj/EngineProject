#pragma once

// Box2D
#include <Box2D/Box2D.h>

// Engine
#include <Engine/Net/Connection.hpp>
#include <Engine/Net/Replication.hpp>

// Game
#include <Game/Common.hpp>
#include <Game/PhysicsSystem.hpp>


namespace Game {
	class PhysicsComponent {
		private:
			friend class PhysicsSystem;
			b2Transform prevTransform;
			b2Transform interpTransform;
			b2Body* body = nullptr;

		public:
			constexpr static auto networkReplication = Engine::Net::Replication::ALWAYS;

			void setBody(b2Body* body); // TODO: add constructor arguments world.addComponent
			b2Body& getBody();
			b2World* getWorld();

			void updateTransform(const b2Transform& trans);
			void updateTransform(const b2Vec2& pos, float32 ang);
			void setTransform(const b2Transform& trans);
			void setTransform(const b2Vec2& pos, float32 ang);
			const b2Vec2& getPosition() const;
			const b2Vec2& getInterpPosition() const;

			void toNetwork(Engine::Net::Connection& conn) const;
			void fromNetwork(Engine::Net::Connection& conn);
	};
}
