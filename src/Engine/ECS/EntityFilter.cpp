// STD
#include <algorithm>

// Engine
#include <Engine/ECS/EntityFilter.hpp>


namespace Engine::ECS {
	void EntityFilter::add(Entity ent, const ComponentBitset& cbits) {
		if ((cbits & componentsBits) == componentsBits) {
			// TODO: Use array style?
			auto pos = std::lower_bound(entities.cbegin(), entities.cend(), ent);
			entities.insert(pos, ent);
		}
	}

	void EntityFilter::remove(Entity ent) {
		auto pos = std::lower_bound(entities.cbegin(), entities.cend(), ent);

		if (pos != entities.cend()) {
			entities.erase(pos);
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
