// Engine
#include <Engine/ECS/EntityFilter.hpp>


namespace Engine::ECS {
	EntityFilter::EntityFilter(ComponentBitset componentBits) : componentBits{componentBits} {
	}

	auto EntityFilter::begin() const -> ConstIterator {
		return entities.cbegin();
	}

	auto EntityFilter::end() const -> ConstIterator {
		return entities.cend();
	}

	void EntityFilter::addEntity(Entity ent) {
		if (entities.size() < ent.id) {
			entities.resize(ent.id + 1);
		}

		entities[ent.id] = std::move(ent);
	}

	Entity EntityFilter::operator[](std::size_t index) const {
		return entities[index];
	}
}
