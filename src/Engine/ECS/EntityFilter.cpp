// STD
#include <algorithm>

// Engine
#include <Engine/ECS/EntityFilter.hpp>


namespace Engine::ECS {
	void EntityFilter::add(Entity ent, const ComponentBitset& cbits) {
		puts("add 2");
		if ((cbits & componentsBits) == componentsBits) {
			// TODO: Use array style?
			auto pos = std::lower_bound(entities.cbegin(), entities.cend(), ent);
			entities.insert(pos, ent);
			puts("add 3");
		}
	}

	auto EntityFilter::begin() -> ConstIterator {
		return entities.cbegin();
	}

	auto EntityFilter::end() -> ConstIterator {
		return entities.cend();
	}

	auto EntityFilter::begin() const -> ConstIterator {
		return entities.cbegin();
	}

	auto EntityFilter::end() const -> ConstIterator {
		return entities.cend();
	}
}
