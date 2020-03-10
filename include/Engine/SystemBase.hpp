#pragma once

// STD
#include <vector>

// Engine
#include <Engine/ECS/Common.hpp>
#include <Engine/ECS/SystemManager.hpp>
#include <Engine/ECS/World.hpp>


namespace Engine {
	/**
	 * A base class from systems.
	 * @tparam World The world type.
	 */
	template<class World>
	class SystemBase : public Engine::ECS::System {
		public:
			/**
			 * Constructor.
			 * @param[in,out] world The world that owns this system.
			 */
			SystemBase(World& world) : world{world} {};

		protected:
			/** The world that owns this system. */
			World& world;
	};
}
