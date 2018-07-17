// Engine
#include <Engine/ECS/EntityFilter.hpp>


namespace Engine::ECS {
	auto EntityFilter::begin() const -> ConstIterator {
		return entities.cbegin();
	}

	auto EntityFilter::end() const -> ConstIterator {
		return entities.cend();
	}

	Entity EntityFilter::operator[](std::size_t index) const {
		return entities[index];
	}
}
