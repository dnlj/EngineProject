#pragma once

// Engine
#include <Engine/Net/Channel.hpp>
#include <Engine/Net/Connection.hpp>

// Game
#include <Game/MessageType.hpp>


namespace Game {
	struct Channel_General : Engine::Net::Channel_UnreliableUnordered<
		MessageType::UNKNOWN,

		MessageType::DISCOVER_SERVER,
		MessageType::SERVER_INFO,
		MessageType::CONNECT_REQUEST,
		MessageType::CONNECT_CHALLENGE,
		MessageType::CONNECT_CONFIRM,
		MessageType::DISCONNECT,

		MessageType::ACTION // TODO: one of these things is not like the others
	> {};

	struct Channel_General_RU : Engine::Net::Channel_ReliableUnordered<
		MessageType::PLAYER_DATA,
		MessageType::ZONE_INFO,
		MessageType::ECS_ZONE_INFO,
		MessageType::SPELL
	> {};

	struct Channel_ECS : Engine::Net::Channel_ReliableOrdered<
		MessageType::CONFIG_NETWORK,
		MessageType::ECS_INIT,
		MessageType::ECS_ENT_CREATE,
		MessageType::ECS_ENT_DESTROY,
		MessageType::ECS_COMP_ADD,
		MessageType::ECS_COMP_ALWAYS,
		MessageType::ECS_FLAG
	> {};

	struct Channel_Map_Blob : Engine::Net::Channel_LargeReliableOrdered<
		MessageType::MAP_CHUNK
	> {};

	using Connection = Engine::Net::Connection<
		Channel_General,
		Channel_General_RU,
		Channel_ECS,

		// One problem with having this last is that we can never use a full packet
		// On the other hand we dont want map data to eat 100% bandwidth and cause
		// jumpy gameplay. Currently the solution is to limit the maximum use of Channel_LargeReliableOrdered.
		// Doing this we need to make sure we never send more than this maximum EVERY frame (such as ACTION messages)
		// or else they will never be sent.
		Channel_Map_Blob 
	>;
}
