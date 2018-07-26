#pragma once

// Engine
#include <Engine/ECS/Common.hpp>

namespace Engine::ECS {
	/**
	 * Abstract class representing a system.
	 */
	class System {
		public:
			/** The bitset of systems to have higher priority than. */
			Engine::ECS::SystemBitset priorityBefore;

			/** The bitset of systems to have lower priority than. */
			Engine::ECS::SystemBitset priorityAfter;

			System() = default;
			System(const System&) = delete;

			virtual ~System() = 0;
			virtual void run(float dt) {};
	};
}
