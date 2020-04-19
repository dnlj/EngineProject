#pragma once

// STD
#include <cstdint>
#include <ostream>

// Engine
#include <Engine/Engine.hpp>


namespace Engine::ECS {
	class Entity {
		public:
			uint16 id;
			uint16 gen;
	};

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
