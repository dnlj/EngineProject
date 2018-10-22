// STD
#include <algorithm>

// Engine
#include <Engine/ECS/EntityFilter.hpp>


namespace Engine::ECS {
	EntityFilter::EntityFilter(const EntityManager& entityManager)
		: entityManager{entityManager} {
	}

	void EntityFilter::add(Entity ent, const ComponentBitset& cbits) {
		if ((cbits & componentsBits) == componentsBits) {
			auto pos = std::lower_bound(entities.begin(), entities.end(), ent);

			#if defined(DEBUG)
				if (pos != entities.end() && *pos == ent) {
					ENGINE_ERROR("Attempting to add duplicate entity to filter");
				}
			#endif

			entities.insert(pos, ent);
		}
	}

	void EntityFilter::remove(Entity ent) {
		auto pos = std::lower_bound(entities.cbegin(), entities.cend(), ent);

		if (pos != entities.cend() && *pos == ent) {
			entities.erase(pos);
		}
	}

	std::size_t EntityFilter::size() const {
		return std::distance(begin(), end());
	}

	bool EntityFilter::empty() const {
		return size() == 0;
	}

	auto EntityFilter::begin() const -> ConstIterator {
		auto it = ConstIterator(*this, entities.begin());

		if (!entityManager.isEnabled(*it)) {
			++it;
		}

		return it;
	}

	auto EntityFilter::end() const -> ConstIterator {
		return ConstIterator(*this, entities.end());
	}

	auto EntityFilter::cbegin() const -> ConstIterator {
		return begin();
	}

	auto EntityFilter::cend() const -> ConstIterator {
		return end();
	}
}
