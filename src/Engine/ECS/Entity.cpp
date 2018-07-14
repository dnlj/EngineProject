// Engine
#include <Engine/ECS/Entity.hpp>


namespace Engine::ECS {
	bool operator==(const Entity& e1, const Entity& e2) {
		return (e1.id == e2.id) && (e1.gen == e2.gen);
	}

	bool operator!=(const Entity& e1, const Entity& e2) {
		return !(e1 == e2);
	}

	bool operator<(const Entity& e1, const Entity& e2) {
		return (e1.gen < e2.gen)
			|| (e1.id < e2.id) && (e1.gen == e2.gen);
	}

	bool operator>=(const Entity& e1, const Entity& e2) {
		return !(e1 < e2);
	}

	bool operator>(const Entity& e1, const Entity& e2) {
		return (e1 != e2) && (e1 >= e2);
	}

	bool operator<=(const Entity& e1, const Entity& e2) {
		return !(e1 > e2);
	}

	std::ostream& operator<<(std::ostream& os, const Entity& ent) {
		os << "Entity(" << ent.id << ", " << ent.gen << ")";
		return os;
	}
}
