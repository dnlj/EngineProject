#pragma once

// GLM
#include <glm/vec2.hpp>

namespace Game {
	class CharacterMovementComponent {
		public:
			constexpr static bool isSnapshotRelevant = true;
			glm::ivec2 dir;
	};
}
