#pragma once

// STD
#include <tuple>

// Game
#include <Game/Common.hpp>


namespace Game {
	struct MessageType {
		enum Type : Engine::Net::MessageType {
			#define X(name) name,
			#include <Game/MessageType.xpp>
			_COUNT,
		};
	};
}
