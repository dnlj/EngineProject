#pragma once

// Engine
#include <Engine/Net/Channel.hpp>
#include <Engine/Net/Connection.hpp>

// Game
#include <Game/MessageType.hpp>


namespace Game {
	struct Channel_General : Engine::Net::Channel_UnreliableUnordered<
		MessageType::DISCOVER_SERVER, // TODO: Move into channel for valid unconnected messages?
		MessageType::SERVER_INFO,	  // TODO: Move into channel for valid unconnected messages?
		MessageType::CONNECT,		  // TODO: Move into channel for valid unconnected messages?
		MessageType::CONNECT_CONFIRM, // TODO: Move into channel for valid unconnected messages?
		MessageType::DISCONNECT,
		MessageType::ACK,
		MessageType::TEST,
		MessageType::ACTION // TODO: one of these things is not like the others
	> {};

	struct Channel_General_RU : Engine::Net::Channel_ReliableUnordered<
		MessageType::PING
	> {};

	struct Channel_ECS : Engine::Net::Channel_UnreliableUnordered<
		MessageType::ECS_ENT_CREATE,
		MessageType::ECS_ENT_DESTROY,
		MessageType::ECS_COMP_ADD,
		MessageType::ECS_COMP_ALWAYS,
		MessageType::ECS_FLAG
	> {};

	using Connection = Engine::Net::Connection<
		Channel_General,
		Channel_General_RU,
		Channel_ECS
	>;
}
