#pragma once

// STD
#include <memory>

// Game
#include <Game/Connection.hpp>


namespace Game {
	class ConnectionComponent {
		public:
			std::unique_ptr<Connection> conn = {};
			Engine::Clock::TimePoint disconnectAt = {};
	};
}
