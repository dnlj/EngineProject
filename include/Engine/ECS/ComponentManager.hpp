#pragma once

// Engine
#include <Engine/ECS/Common.hpp>

namespace Engine::ECS {
	class ComponentManager {
		public:
		private:
			/** The next id to use for components. */
			ComponentID nextComponentID = 0;	

			/**
			 * @brief Gets the next ComponentID.
			 * @return The next ComponentID.
			 */
			ComponentID getNextComponentID();
	};
}