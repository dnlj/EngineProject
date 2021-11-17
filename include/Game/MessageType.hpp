#pragma once

// STD
#include <tuple>

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

	template<MessageType::Type type>
	struct MessageType_Traits {
	};

	#define X(Name, Side, State)\
	template<> struct MessageType_Traits<MessageType::Name> {\
		constexpr static auto side = Side;\
		constexpr static auto state = State;\
	};
	#include <Game/MessageType.xpp>
}
