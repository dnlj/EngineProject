// STD
#include <algorithm>

// Engine
#include <Engine/ECS/EntityFilter.hpp>


namespace Engine::ECS {
	template<class T>
	EntityFilter::Iterator<T>::Iterator(const EntityFilter& filter, ItType it)
		: filter(filter)
		, it(it) {
	}

	template<class T>
	T& EntityFilter::Iterator<T>::operator*() {
		return **const_cast<const Iterator*>(this);
	};

	template<class T>
	T& EntityFilter::Iterator<T>::operator*() const {
		return *it;
	}

	template<class T>
	T* EntityFilter::Iterator<T>::operator->() {
		return const_cast<const Iterator*>(this)->operator->();
	}

	template<class T>
	T* EntityFilter::Iterator<T>::operator->() const {
		return &*it;
	}

	template<class T>
	auto EntityFilter::Iterator<T>::operator++() -> Iterator& {
		auto end = filter.end();

		#if defined(DEBUG)
			if (*this == end) {
				ENGINE_ERROR("Attempting to increment an end iterator");
			}
		#endif

		while ((++it, *this) != end && !filter.entityManager.isEnabled(*it)) {
		}

		return *this;
	}

	template<class T>
	auto EntityFilter::Iterator<T>::operator--() -> Iterator& {
		auto begin = filter.begin();

		#if defined(DEBUG)
			if (*this == begin) {
				ENGINE_ERROR("Attempting to decrement an begin iterator");
			}
		#endif

		while ((--it, *this) != begin && !filter.entityManager.isEnabled(*it)) {
		}

		return *this;
	}

	template<class T>
	auto EntityFilter::Iterator<T>::operator++(int) -> Iterator {
		auto temp = *this;
		++*this;
		return temp;
	}

	template<class T>
	auto EntityFilter::Iterator<T>::operator--(int) -> Iterator {
		auto temp = *this;
		--*this;
		return temp;
	}
}

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
