#pragma once

// STD
#include <tuple>

// Game
#include <Game/Common.hpp>


namespace Game {
	struct MessageType {
		enum Type : Engine::Net::MessageType {
			UNKNOWN,
			DISCOVER_SERVER,
			SERVER_INFO,
			CONNECT,
			DISCONNECT,
			PING,
			ECS_COMP,
			ACTION,
			ACK,
			// TODO: static asserts to make sure support structures are defined, such as dispatchMessage HANDLE cases
			_COUNT, 
		};
	};
}
