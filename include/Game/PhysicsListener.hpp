#pragma once

// Engine
#include <Engine/ECS/Entity.hpp>


namespace Game {
	class PhysicsListener {
		public:
			/**
			 * Destructor
			 */
			virtual ~PhysicsListener() {};

			/**
			 * Called two entities begin contact.
			 */
			virtual void beginContact(const Engine::ECS::Entity& entA, const Engine::ECS::Entity& entB) {};

			/**
			 * Called two entities end contact.
			 */
			virtual void endContact(const Engine::ECS::Entity& entA, const Engine::ECS::Entity& entB) {};

			//TODO: virtual void preSolve() {};

			//TODO: virtual void postSolve() {};
	};
}
