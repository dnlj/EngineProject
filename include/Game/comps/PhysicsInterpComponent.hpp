#pragma once

// Box2D
#include <box2d/b2_body.h>


namespace Game {
	class PhysicsInterpComponent {
		//private:
		public:
			b2Transform trans = {};

		public:
			const auto& getPosition() const { return trans.p; }
	};
}
