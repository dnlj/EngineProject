#pragma once

// Engine
#include <Engine/Net/Connection.hpp>

namespace Game {
	// TODO: xmacro this stuff
	#define CHANNEL(Name, Flags) struct Name##_Tag : ::Engine::Net::Channel2<Flags> {} constexpr Name;
	CHANNEL(TestChannel1, Engine::Net::ChannelFlags::Reliable | Engine::Net::ChannelFlags::Ordered);
	CHANNEL(TestChannel2, Engine::Net::ChannelFlags::Reliable | Engine::Net::ChannelFlags::Ordered);
	using Connection = Engine::Net::Connection2<
		TestChannel1_Tag,
		TestChannel2_Tag
	>;
	#undef CHANNEL
}
