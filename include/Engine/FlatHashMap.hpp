#pragma once

// RHH
#include <robin_hood.h>


namespace Engine {
	template <
		class KeyType,
		class ValueType,
		size_t MaxLoadFactor100 = 75,
		class Equal = std::equal_to<KeyType>,
		class Hash = robin_hood::hash<KeyType>
	>
	using FlatHashMap = robin_hood::unordered_flat_map<KeyType, ValueType, Hash, Equal, MaxLoadFactor100>;
}
