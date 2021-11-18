#pragma once

// Game
#include <Game/Common.hpp>


namespace Game {
	struct ConnectionState_ {
		enum ConnectionState : uint8 {
			None          = 0,
			Unconnected   = 1 << 0, // Has not connected
			Connecting    = 1 << 1, // Is in the process of connecting
			Connected     = 1 << 2, // Has fully connected
			Disconnecting = 1 << 3, // Is in the process of disconnecting
			Disconnected  = 1 << 4, // Has fully disconnected
			Any           = 0xFF,
		};
	};
	using ConnectionState = ConnectionState_::ConnectionState;

	struct MessageType_ {
		enum MessageType : Engine::Net::MessageType {
			#define X(Name, Side, State) Name,
			#include <Game/MessageType.xpp>
			_count,
		};
	};
	using MessageType = MessageType_::MessageType;
}

#define X(Name, Side, State)\
template<> struct Engine::Net::MessageTraits<Game::MessageType::Name> {\
	using ConnectionState = Game::ConnectionState;\
	constexpr static auto side = Side;\
	constexpr static auto state = State;\
	constexpr static char name[] = #Name;\
};
#include <Game/MessageType.xpp>


namespace Game {
	inline std::string_view getMessageName(Engine::Net::MessageType msg) {
		if (msg < 0 || msg >= MessageType::_count) { return ""; }

		#define X(Name, Side, State) case MessageType::Name: { return Engine::Net::MessageTraits<MessageType::Name>::name; }
		switch(msg) {
			#include <Game/MessageType.xpp>
		}

		// Should never be hit
		return "THIS IS A BUG. NO MESSAGE NAME FOUND.";
	}
}
