#pragma once

// GLM
#include <glm/vec2.hpp>

// Game
#include <Game/System.hpp>


namespace Game {
	class PhysicsOriginShiftSystem : public System {
		public:
			// Somewhat arbitrary. Small enough to not loose precision. Large enough to not be constantly shifting.
			//constexpr static float32 range = 8000.0f; // Gives us .0005 precision (values < 8192)
			constexpr static float32 range = 32.0f; // TODO: rm - temp for testing. use above

		private:
			/** Current number of times offset in each direction */
			glm::ivec2 offset = {0, 0};

		public:
			PhysicsOriginShiftSystem(SystemArg arg);

			void update(float32 dt);

			glm::ivec2 getOffset() const;
	};
}
