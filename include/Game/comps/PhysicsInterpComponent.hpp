#pragma once

// Box2D
#include <box2d/b2_body.h>


namespace Game {
	class PhysicsInterpComponent {
		//private:
		public:
			b2Transform trans = {};

		public:
			bool onlyUserVerified = false;
			const auto& getPosition() const { return trans.p; }
			constexpr static Engine::Net::Replication netRepl() { return Engine::Net::Replication::ONCE; };

			ENGINE_INLINE void netTo(Connection& conn) const {};
			ENGINE_INLINE void netToInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Connection& conn) const {};
			ENGINE_INLINE void netFrom(Connection& conn) {};
			ENGINE_INLINE void netFromInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Connection& conn) {
				onlyUserVerified = true;
			};
	};
}
