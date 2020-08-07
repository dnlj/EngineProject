#pragma once

// STD
#include <tuple>

// Game
#include <Game/Common.hpp>


namespace Game {
	struct MessageType {
		enum Type : Engine::Net::MessageType {
			#define X(Name, Side, State) Name,
			#include <Game/MessageType.xpp>
			_COUNT,
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
