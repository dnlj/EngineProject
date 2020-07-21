#pragma once

// Engine
#include <Engine/Net/Connection.hpp>

namespace Game {
	// TODO: xmacro this stuff
	#define CHANNEL(Name, Flags) struct Name##_Tag : ::Engine::Net::Channel2<Flags> {} constexpr Name;
	CHANNEL(General_UU, Engine::Net::ChannelFlags::None);
	CHANNEL(General_RU, Engine::Net::ChannelFlags::Reliable);
	CHANNEL(General_RO, Engine::Net::ChannelFlags::Reliable | Engine::Net::ChannelFlags::Ordered);

	using Connection = Engine::Net::Connection2<
		General_UU_Tag,
		General_RU_Tag,
		General_RO_Tag
	>;
	#undef CHANNEL
}
