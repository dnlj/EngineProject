#pragma once

// Box2D
#include <box2d/b2_body.h>

// Engine
#include <Engine/Net/Connection.hpp>
#include <Engine/Net/Replication.hpp>

// Game
#include <Game/Connection.hpp>


namespace Game {
	class PhysicsInterpComponent {
		//private:
		public:
			b2Transform trans = {};

			// TODO: these should really be their own component.
			b2Transform nextTrans;
			Engine::Clock::TimePoint nextTime;
			b2Transform prevTrans;
			Engine::Clock::TimePoint prevTime;
			ENGINE_INLINE float64 calcInterpValue(Engine::Clock::TimePoint t) const noexcept {
				const auto diff = nextTime - prevTime;
				return (t - prevTime).count() / static_cast<float64>(diff.count());
			}

		public:
			bool onlyUserVerified = false;
			const auto& getPosition() const { return trans.p; }
			constexpr static Engine::Net::Replication netRepl() { return Engine::Net::Replication::ONCE; };

			ENGINE_INLINE void netTo(Engine::Net::BufferWriter& buff) const {};
			ENGINE_INLINE void netToInit(EngineInstance& engine, World& world, Engine::ECS::Entity ent, Engine::Net::BufferWriter& buff) const {
				//buff.write(onlyUserVerified);
			};

			ENGINE_INLINE void netFrom(Connection& conn) {};
			ENGINE_INLINE void netFromInit(EngineInstance& engine, World& world, Engine::ECS::Entity ent, Connection& conn) {
				//onlyUserVerified = conn.read<bool>();
				onlyUserVerified = true;
			};
	};
}
