#pragma once

// Box2D
#include <Box2D/Box2D.h>

// Game
#include <Game/Common.hpp>
#include <Game/PhysicsSystem.hpp>


namespace Game {
	class PhysicsComponent {
		private:
			friend class PhysicsSystem;
			b2Transform prevTransform;
			b2Transform interpTransform;

		public:
			// TODO: Should we wrap body so we never directly interact with box2d?
			// TODO: We should so we dont forget to do things like use `setTransform` instead of `body->SetTransform`
			b2Body* body = nullptr;

			void setTransform(const b2Vec2& pos, float32 ang);

			const b2Vec2& getInterpPosition() const;

	};
}
