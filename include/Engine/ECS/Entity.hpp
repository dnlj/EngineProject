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

	bool operator==(const Entity& e1, const Entity& e2);
	bool operator!=(const Entity& e1, const Entity& e2);
	bool operator<(const Entity& e1, const Entity& e2);
	bool operator>=(const Entity& e1, const Entity& e2);
	bool operator>(const Entity& e1, const Entity& e2);
	bool operator<=(const Entity& e1, const Entity& e2);

	std::ostream& operator<<(std::ostream& os, const Entity& ent);
}
