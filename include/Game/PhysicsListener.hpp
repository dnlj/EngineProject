#pragma once


// Game
#include <Game/PhysicsUserData.hpp>


namespace Game {
	class PhysicsListener {
		public:
			/**
			 * Destructor
			 */
			virtual ~PhysicsListener() {};

			/**
			 * Called when contact begins.
			 * @param[in] dataA The first user data.
			 * @param[in] dataB The second user data.
			 */
			virtual void beginContact(const PhysicsUserData& dataA, const PhysicsUserData& dataB) {};

			/**
			 * Called when contact ends.
			 * @param[in] dataA The first user data.
			 * @param[in] dataB The second user data.
			 */
			virtual void endContact(const PhysicsUserData& dataA, const PhysicsUserData& dataB) {};

			//TODO: virtual void preSolve() {};

			//TODO: virtual void postSolve() {};
	};
}
