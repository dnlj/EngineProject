#pragma once

// STD
#include <tuple>

// Game
#include <Game/Common.hpp>
#include <Game/RenderLayer.hpp>
#include <Game/comps/NetworkComponent.hpp>

// Engine
#include <Engine/ArrayView.hpp>
#include <Engine/SparseSet.hpp>


namespace Game {
	class World;
	class EngineInstance;

	/** The argument type system constructors require. */
	using SystemArg = const std::tuple<World&, EngineInstance&>&;

	/**
	 * A base class for game systems.
	 */
	class System {
		public:
			// A view over a subsection of the networkable players.
			// I.e. a subsection of SingleComponentFilter<true, NetworkComponent, World>.
			// I.e. a subsection of the dense part of the sparse set.
			using NetPlySet = Engine::ArrayView<Engine::SparseSet<Engine::ECS::Entity, NetworkComponent>::Iterator::value_type>;

		protected:
			World& world;
			EngineInstance& engine;

		public:
			System(SystemArg arg)
				: world{std::get<World&>(arg)}
				, engine{std::get<EngineInstance&>(arg)} {
			};

			// It is almost certainly a bug if this is hit
			System(const System&) = delete;

			ENGINE_INLINE void setup() {}

			ENGINE_INLINE void preTick() {}
			ENGINE_INLINE void tick() {}
			ENGINE_INLINE void postTick() {}

			ENGINE_INLINE void update(const float32 dt) {}
			ENGINE_INLINE void render(const RenderLayer layer) {} // Added by RenderPassSystem
			ENGINE_INLINE void network(const NetPlySet plys) {} // Added by NetworkingSystem

			ENGINE_INLINE void preStoreSnapshot() {}
			ENGINE_INLINE void postLoadSnapshot() {}
	};
}
