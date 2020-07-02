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
			void setBody(b2Body* body); // TODO: add constructor arguments world.addComponent
			b2Body& getBody();
			b2World* getWorld();

			void updateTransform(const b2Transform& trans);
			void updateTransform(const b2Vec2& pos, float32 ang);
			void setTransform(const b2Transform& trans);
			void setTransform(const b2Vec2& pos, float32 ang);
			const b2Vec2& getPosition() const;
			const b2Vec2& getInterpPosition() const;

			Engine::Net::Replication netRepl() const;
			void netTo(Engine::Net::PacketWriter& writer) const;
			void netToInit(World& world, Engine::ECS::Entity ent, Engine::Net::PacketWriter& writer) const;
			void netFrom(Engine::Net::PacketReader& reader);
			void netFromInit(World& world, Engine::ECS::Entity ent, Engine::Net::PacketReader& reader);
	};
}
