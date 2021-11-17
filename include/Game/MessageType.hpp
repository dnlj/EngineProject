#pragma once

// Game
#include <Game/Common.hpp>


namespace Game {
	struct ConnectionState_ {
		enum ConnectionState : uint8 {
			None         = 0,
			Disconnected  = 1 << 0,
			Connecting    = 1 << 1,
			Connected     = 1 << 2,
			Any           = Disconnected | Connecting | Connected,
		};
	};
	using ConnectionState = ConnectionState_::ConnectionState;

	// TODO: change to struct,enum,using style
	struct MessageType {
		enum Type : Engine::Net::MessageType {
			#define X(Name, Side, State) Name,
			#include <Game/MessageType.xpp>
			_count,
		};
	};
}

#define X(Name, Side, State)\
template<> struct Engine::Net::MessageTraits<Game::MessageType::Name> {\
	using ConnectionState = Game::ConnectionState;\
	constexpr static auto side = Side;\
	constexpr static auto state = State;\
	constexpr static char name[] = #Name;\
};
#include <Game/MessageType.xpp>
