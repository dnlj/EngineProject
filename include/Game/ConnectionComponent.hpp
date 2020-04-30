#pragma once

// STD
#include <memory>

// Engine
#include <Engine/Net/MessageStream.hpp>
#include <Engine/Net/Connection.hpp>


namespace Game {
	class ConnectionComponent {
		public:
			// TODO: work around for not allowing components without a default constructor. Remove unique_ptr once ECS rework is finished
			std::unique_ptr<Engine::Net::Connection> conn;
	};
}
