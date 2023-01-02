#pragma once

// Game
#include <Game/Common.hpp>


namespace Game {
	struct ConnectionState_ {
		enum ConnectionState : Engine::Net::ConnectionState {
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

namespace Game {
	inline const Engine::Net::MessageMetaInfo& getMessageMetaInfo(Engine::Net::MessageType mtype) {
		using enum Game::ConnectionState;
		constexpr static Engine::Net::MessageMetaInfo infos[Game::MessageType::_count + 1] = {
			#define X(Name, Dir, SendState, RecvState) {\
				.dir = Engine::Net::MessageDirection::Dir,\
				.sendState = SendState,\
				.recvState = RecvState,\
				.name = #Name,\
			},
			#include <Game/MessageType.xpp>
			{
				.dir = Engine::Net::MessageDirection::None,
				.sendState = ConnectionState::None,
				.recvState = ConnectionState::None,
				.name = "#INVALID_MESSAGE_TYPE#",
			}
		};

		if (mtype < 0 || mtype >= MessageType::_count) {
			ENGINE_DEBUG_ASSERT(false, "Attempting to get meta info of invalid message type.");
			return infos[Game::MessageType::_count];
		}

		return infos[mtype];
	};
}


// TODO: really we should work on getting rid of this when we do the connection rework, the Engine::Connection really shouldnt need this info, or get it in a more flexible way. This feels very specific to how we are currently doing things.
#define X(Name, Side, SState, RState)\
template<> inline const Engine::Net::MessageMetaInfo& Engine::Net::getMessageMetaInfo<Game::MessageType::Name>() { return Game::getMessageMetaInfo(Game::MessageType::Name); }
#include <Game/MessageType.xpp>
