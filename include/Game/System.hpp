#pragma once

// STD
#include <tuple>

// Engine
#include <Engine/EngineInstance.hpp>


namespace Game {
	class World; // Forward declaration

	using SystemArg = const std::tuple<World&, Engine::EngineInstance&>&;

	/**
	 * A base class from systems.
	 */
	class SystemBase {
		protected:
			World& world;
			Engine::EngineInstance& engine;

		public:
			/**
			 * Constructor.
			 * @param[in,out] world The world that owns this system.
			 */
			SystemBase(SystemArg arg)
				: world{std::get<World&>(arg)}
				, engine{std::get<Engine::EngineInstance&>(arg)} {
			};
	};
}
