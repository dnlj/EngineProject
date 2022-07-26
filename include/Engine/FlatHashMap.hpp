#pragma once

// RHH
#include <robin_hood.h>

// Engine
#include <Engine/Hash.hpp>
#include <Engine/EqualTo.hpp>


namespace Engine {
	template <
		class KeyType,
		class ValueType,
		class Hash = Hash<KeyType>,
		class Equal = EqualTo<KeyType>,
		size_t MaxLoadFactor100 = 75
	>
	using FlatHashMap = robin_hood::unordered_flat_map<KeyType, ValueType, Hash, Equal, MaxLoadFactor100>;
}
