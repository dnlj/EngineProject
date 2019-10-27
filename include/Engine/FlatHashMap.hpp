#pragma once

// RHH
#include <robin_hood.h>

// Engine
#include <Engine/Hash.hpp>


namespace Engine {
	// TODO: put in `Container` namespace?
	template <
		class KeyType,
		class ValueType,
		class Hash = Engine::Hash<KeyType>,
		class Equal = std::equal_to<KeyType>,
		size_t MaxLoadFactor100 = 75
	>
	using FlatHashMap = robin_hood::unordered_flat_map<KeyType, ValueType, Hash, Equal, MaxLoadFactor100>;
}
