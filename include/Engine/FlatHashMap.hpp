#pragma once

// RHH
#include<robin_hood.h>

// Engine
#include <Engine/Hash.hpp>
#include <Engine/EqualTo.hpp>


namespace Engine {
	template<
		class KeyType,
		class ValueType,
		class Hash = Hash<KeyType>,
		class Equal = EqualTo<KeyType>,
		size_t MaxLoadFactor100 = 75
	>
	#if 1 // For debugging, std implementations have more checking than robin_hood does.
		using FlatHashMap = robin_hood::unordered_flat_map<KeyType, ValueType, Hash, Equal, MaxLoadFactor100>;
	#else
		#if ENGINE_RELEASE
			#error This should not be set in release mode.
		#endif
		using FlatHashMap = std::unordered_map<KeyType, ValueType, Hash, Equal>;
	#endif
		
	template<
		class KeyType,
		class Hash = Hash<KeyType>,
		class Equal = EqualTo<KeyType>,
		size_t MaxLoadFactor100 = 75
	>
	using FlatHashSet = robin_hood::unordered_flat_set<KeyType, Hash, Equal, MaxLoadFactor100>;
}
