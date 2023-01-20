// Engine
#include <Engine/ECS/Entity.hpp>


namespace Engine::ECS {
	std::ostream& operator<<(std::ostream& os, Entity ent) {
		os << "Entity(" << ent.id << ", " << ent.gen << ")";
		return os;
	}
}
