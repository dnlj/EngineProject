#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Bitset.hpp>


namespace Engine::Net {
	using MessageType = uint8;
	using SeqNum = uint16;

	using AckBitset = Bitset<64, uint64>;
}
