#pragma once


// Game
#include <Game/PhysicsUserData.hpp>


namespace Game {
	// TODO: Doc
	class PhysicsListener {
		public:
			virtual ~PhysicsListener() {};

			virtual void beginContact(const PhysicsUserData& dataA, const PhysicsUserData& dataB) {};
			virtual void endContact(const PhysicsUserData& dataA, const PhysicsUserData& dataB) {};
			//TODO: virtual void preSolve() {};
			//TODO: virtual void postSolve() {};
	};
}
