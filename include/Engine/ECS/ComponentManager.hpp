#pragma once

// STD
#include <array>

// Engine
#include <Engine/ECS/Common.hpp>


// TODO: Document
// TODO: Test
namespace Engine::ECS {
	class ComponentManager {
		public:
			/**
			 * @brief Gets the next ComponentID.
			 * @return The next ComponentID.
			 */
			ComponentID getNextComponentID();

		private:
			ComponentID nextID = 0;
	};
}