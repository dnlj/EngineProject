#pragma once

// STD
#include <memory>

// Game
#include <Game/Connection.hpp>


namespace Game {
	class ConnectionComponent {
		public:
			// TODO: does this still need to be in a ptr?
			std::unique_ptr<Connection> conn;
	};
}
