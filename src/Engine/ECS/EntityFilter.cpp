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

		if (pos != entities.cend() && *pos == ent) {
			entities.erase(pos);
		}
	}

	bool EntityFilter::empty() const {
		return entities.empty();
	}

	std::size_t EntityFilter::size() const {
		return entities.size();
	}

	auto EntityFilter::begin() const -> ConstIterator {
		return ConstIterator(entities.begin()); // TODO: impl
	}

	auto EntityFilter::end() const -> ConstIterator {
		return ConstIterator(entities.end()); // TODO: impl
	}

	auto EntityFilter::cbegin() const -> ConstIterator {
		return begin();
	}

	auto EntityFilter::cend() const -> ConstIterator {
		return end();
	}
}
