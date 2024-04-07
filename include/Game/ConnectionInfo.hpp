#pragma once

// Game
#include <Game/Connection.hpp>

namespace Game {
	class ConnectionInfo : public Connection {
		public:
			using Connection::Connection;
			Engine::ECS::Entity ent{}; // TODO: probably add getter/setter once conversion is done. Makes access cleaner
			Engine::Clock::TimePoint disconnectAt = {};
	};
}
