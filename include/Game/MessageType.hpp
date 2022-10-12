#pragma once

// Game
#include <Game/Common.hpp>


namespace Game {
	struct ConnectionState_ {
		enum ConnectionState : uint8 {
			None          = 0,
			Disconnected  = 1 << 0, // Not connected
			Connecting    = 1 << 1, // Is in the process of connecting
			Connected     = 1 << 2, // Has fully connected
			Disconnecting = 1 << 3, // Is in the process of disconnecting
			Any           = 0xFF,
		};
	};
	using ConnectionState = ConnectionState_::ConnectionState;

	struct MessageType_ {
		enum MessageType : Engine::Net::MessageType {
			#define X(Name, Side, SState, RState) Name,
			#include <Game/MessageType.xpp>
			_count,
		};
	};
	using MessageType = MessageType_::MessageType;
}

#define X(Name, Side, SState, RState)\
template<> struct Engine::Net::MessageTraits<Game::MessageType::Name> {\
	using enum Game::ConnectionState;\
	constexpr static auto side = Side;\
	constexpr static auto sstate = SState;\
	constexpr static auto rstate = RState;\
	constexpr static char name[] = #Name;\
};
#include <Game/MessageType.xpp>


namespace Game {
	inline std::string_view getMessageName(Engine::Net::MessageType msg) {
		if (msg < 0 || msg >= MessageType::_count) { return ""; }

		#define X(Name, Side, SState, RState) case MessageType::Name: { return Engine::Net::MessageTraits<MessageType::Name>::name; }
		switch(msg) {
			#include <Game/MessageType.xpp>
		}

		// Should never be hit
		return "THIS IS A BUG. NO MESSAGE NAME FOUND.";
	}
}
