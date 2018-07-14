#pragma once

// STD
#include <cstdint>
#include <ostream>


namespace Engine::ECS {
	class Entity {
		public:
			std::uint32_t id;
			std::uint16_t gen;
	};

	bool operator==(const Entity& e1, const Entity& e2);
	bool operator!=(const Entity& e1, const Entity& e2);
	bool operator<(const Entity& e1, const Entity& e2);
	bool operator>=(const Entity& e1, const Entity& e2);
	bool operator>(const Entity& e1, const Entity& e2);
	bool operator<=(const Entity& e1, const Entity& e2);

	std::ostream& operator<<(std::ostream& os, const Entity& ent);
}
