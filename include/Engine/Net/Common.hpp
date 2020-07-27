#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Bitset.hpp>


namespace Engine::Net {
	using MessageType = uint8;
	using SeqNum = uint32; // TODO: look into warpped seq nums

	using AckBitset = Bitset<64, uint64>;
}
