#pragma once

// STD
#include <memory>

// Engine
#include <Engine/Net/MessageStream.hpp>
#include <Engine/Net/Connection.hpp>


namespace Game {
	class ConnectionComponent {
		public:
			std::unique_ptr<Engine::Net::Connection> conn;
	};
}
