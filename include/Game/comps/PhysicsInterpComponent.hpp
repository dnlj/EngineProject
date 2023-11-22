#pragma once

// Box2D
#include <box2d/b2_body.h>

// Game
#include <Game/NetworkTraits.hpp>


namespace Game {
	class PhysicsInterpComponent {
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
	};

	template<>
	class NetworkTraits<PhysicsInterpComponent> {
		public:
			static Engine::Net::Replication getReplType(const PhysicsInterpComponent& obj) {
				return Engine::Net::Replication::ONCE;
			}

			static void writeInit(const PhysicsInterpComponent& obj, Engine::Net::BufferWriter& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent) {
			}

			static void write(const PhysicsInterpComponent& obj, Engine::Net::BufferWriter& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent) {
			}

			static std::tuple<PhysicsInterpComponent> readInit(Engine::Net::BufferReader& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent) {
				PhysicsInterpComponent result;
				result.onlyUserVerified = true;
				return result;
			}

			static void read(PhysicsInterpComponent& obj, Engine::Net::BufferReader& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent) {
			}
	};
}
