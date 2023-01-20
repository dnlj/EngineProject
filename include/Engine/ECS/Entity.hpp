#pragma once

// STD
#include <iosfwd>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Hash.hpp>
#include <Engine/IndexHash.hpp>


namespace Engine::ECS {
	class Entity {
		public:
			// TODO (yClymPoU): make Entity(0,0) the invalid entity (with gen=0 invalid)
			uint16 id = static_cast<uint16>(-1);
			uint16 gen = static_cast<uint16>(-1);

			ENGINE_INLINE friend bool operator==(Entity lhs, Entity rhs) { return (lhs.id == rhs.id) && (lhs.gen == rhs.gen); }
			ENGINE_INLINE friend bool operator!=(Entity lhs, Entity rhs) { return !(lhs == rhs); }
			ENGINE_INLINE friend bool operator<(Entity lhs, Entity rhs) { return (lhs.gen < rhs.gen) || (lhs.id < rhs.id) && (lhs.gen == rhs.gen); }
			ENGINE_INLINE friend bool operator>=(Entity lhs, Entity rhs) { return !(lhs < rhs); }
			ENGINE_INLINE friend bool operator>(Entity lhs, Entity rhs) { return (lhs != rhs) && (lhs >= rhs); }
			ENGINE_INLINE friend bool operator<=(Entity lhs, Entity rhs) { return !(lhs > rhs); }
			ENGINE_INLINE explicit operator bool() const { return *this != Entity{}; }
	};

	static_assert(sizeof(Entity) == 4);
	static_assert(sizeof(Entity) <= sizeof(void*),
		"Since entities are sometimes stored in userdata pointers, they should not exceed the size of a pointer."
	);

	constexpr Entity INVALID_ENTITY = {};

	std::ostream& operator<<(std::ostream& os, Entity ent);
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
