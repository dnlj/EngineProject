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
	 * This class provides functions for handling the addition and remove of entities that are suitable for most systems.
	 * @tparam World The world type.
	 */
	template<class World>
	class SystemBase;

	/** @see SystemBase */
	template<class SystemsSet, class ComponentsSet>
	class SystemBase<ECS::World<SystemsSet, ComponentsSet>> : public Engine::ECS::System {
		public:
			/** The world type used by this system. */
			using World = ECS::World<SystemsSet, ComponentsSet>;

			/**
			 * Constructor.
			 * @param[in,out] world The world that owns this system.
			 */
			SystemBase(World& world);

		protected:
			/** The world that owns this system. */
			World& world;
	};
}

#include <Engine/SystemBase.ipp>
