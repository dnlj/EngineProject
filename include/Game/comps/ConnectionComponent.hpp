#pragma once

// STD
#include <memory>

// Engine
#include <Engine/Net/Connection.hpp>

// Game
#include <Game/Connection.hpp>


namespace Game {
	class ConnectionComponent {
		public:
			std::unique_ptr<Engine::Net::Connection> conn;
			Connection conn2;
	};
}
