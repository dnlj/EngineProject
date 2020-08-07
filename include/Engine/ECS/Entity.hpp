#pragma once

// STD
#include <cstdint>
#include <ostream>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Hash.hpp>
#include <Engine/IndexHash.hpp>


namespace Engine::ECS {
	class Entity {
		public:
			uint16 id = static_cast<uint16>(-1);
			uint16 gen = static_cast<uint16>(-1);
	};

	static_assert(sizeof(Entity) == 4);
	static_assert(sizeof(Entity) <= sizeof(void*),
		"Since entities are sometimes stored in userdata pointers, they should not exceed the size of a pointer."
	);

	constexpr Entity INVALID_ENTITY = {-1, -1};

	bool operator==(const Entity& e1, const Entity& e2);
	bool operator!=(const Entity& e1, const Entity& e2);
	bool operator<(const Entity& e1, const Entity& e2);
	bool operator>=(const Entity& e1, const Entity& e2);
	bool operator>(const Entity& e1, const Entity& e2);
	bool operator<=(const Entity& e1, const Entity& e2);

	std::ostream& operator<<(std::ostream& os, const Entity& ent);
}

namespace Engine {
	template<> struct Hash<ECS::Entity> {
		size_t operator()(const ECS::Entity& val) const {
			return reinterpret_cast<const uint32&>(val);
		}
	};

	template<> struct IndexHash<ECS::Entity> {
		int32 operator()(const ECS::Entity& v) const {
			return v.id;
		}
	};
}
