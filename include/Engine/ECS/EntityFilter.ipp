#pragma once


namespace Engine::ECS {
	template<class World>
	template<class T>
	EntityFilter<World>::Iterator<T>::Iterator(const EntityFilter& filter, ItType it)
		: filter(filter)
		, it(it) {
	}

	template<class World>
	template<class T>
	T& EntityFilter<World>::Iterator<T>::operator*() {
		return **const_cast<const Iterator*>(this);
	};

	template<class World>
	template<class T>
	T& EntityFilter<World>::Iterator<T>::operator*() const {
		return *it;
	}

	template<class World>
	template<class T>
	T* EntityFilter<World>::Iterator<T>::operator->() {
		return const_cast<const Iterator*>(this)->operator->();
	}

	template<class World>
	template<class T>
	T* EntityFilter<World>::Iterator<T>::operator->() const {
		return &*it;
	}

	template<class World>
	template<class T>
	auto EntityFilter<World>::Iterator<T>::operator++() -> Iterator& {
		auto end = filter.end();

		#if defined(DEBUG)
			if (*this == end) {
				ENGINE_ERROR("Attempting to increment an end iterator");
			}
		#endif

		while ((++it, *this) != end && !filter.world.isEnabled(*it)) {
		}

		return *this;
	}

	template<class World>
	template<class T>
	auto EntityFilter<World>::Iterator<T>::operator--() -> Iterator& {
		auto begin = filter.begin();

		#if defined(DEBUG)
			if (*this == begin) {
				ENGINE_ERROR("Attempting to decrement an begin iterator");
			}
		#endif

		while ((--it, *this) != begin && !filter.world.isEnabled(*it)) {
		}

		return *this;
	}

	template<class World>
	template<class T>
	auto EntityFilter<World>::Iterator<T>::operator++(int) -> Iterator {
		auto temp = *this;
		++*this;
		return temp;
	}

	template<class World>
	template<class T>
	auto EntityFilter<World>::Iterator<T>::operator--(int) -> Iterator {
		auto temp = *this;
		--*this;
		return temp;
	}
	
	template<class World>
	EntityFilter<World>::EntityFilter(const World& world)
		: world{world} {
	}
	
	template<class World>
	void EntityFilter<World>::add(Entity ent, const ComponentBitset& cbits) {
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
	
	template<class World>
	void EntityFilter<World>::remove(Entity ent) {
		auto pos = std::lower_bound(entities.cbegin(), entities.cend(), ent);

		if (pos != entities.cend() && *pos == ent) {
			entities.erase(pos);
		}
	}
	
	template<class World>
	std::size_t EntityFilter<World>::size() const {
		return std::distance(begin(), end());
	}
	
	template<class World>
	bool EntityFilter<World>::empty() const {
		return size() == 0; // TODO: begin != end would be cheaper. Is there a reason we did this? i dont think so.
	}
	
	template<class World>
	auto EntityFilter<World>::begin() const -> ConstIterator {
		auto it = ConstIterator(*this, entities.begin());

		if (!entities.empty() && !world.isEnabled(*it)) {
			++it;
		}

		return it;
	}
	
	template<class World>
	auto EntityFilter<World>::end() const -> ConstIterator {
		return ConstIterator(*this, entities.end());
	}
	
	template<class World>
	auto EntityFilter<World>::cbegin() const -> ConstIterator {
		return begin();
	}
	
	template<class World>
	auto EntityFilter<World>::cend() const -> ConstIterator {
		return end();
	}
}
