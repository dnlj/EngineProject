#pragma once


namespace Engine::ECS {
	template<class Snap>
	template<class T>
	EntityFilter<Snap>::Iterator<T>::Iterator(const EntityFilter& filter, ItType it)
		: filter(filter)
		, it(it) {
	}

	template<class Snap>
	template<class T>
	T& EntityFilter<Snap>::Iterator<T>::operator*() {
		return **const_cast<const Iterator*>(this);
	};

	template<class Snap>
	template<class T>
	T& EntityFilter<Snap>::Iterator<T>::operator*() const {
		return *it;
	}

	template<class Snap>
	template<class T>
	T* EntityFilter<Snap>::Iterator<T>::operator->() {
		return const_cast<const Iterator*>(this)->operator->();
	}

	template<class Snap>
	template<class T>
	T* EntityFilter<Snap>::Iterator<T>::operator->() const {
		return &*it;
	}

	template<class Snap>
	template<class T>
	auto EntityFilter<Snap>::Iterator<T>::operator++() -> Iterator& {
		auto end = filter.end();

		#if defined(DEBUG)
			if (*this == end) {
				ENGINE_ERROR("Attempting to increment an end iterator");
			}
		#endif

		while ((++it, *this) != end && !filter.snap.isEnabled(*it)) {
		}

		return *this;
	}

	template<class Snap>
	template<class T>
	auto EntityFilter<Snap>::Iterator<T>::operator--() -> Iterator& {
		auto begin = filter.begin();

		#if defined(DEBUG)
			if (*this == begin) {
				ENGINE_ERROR("Attempting to decrement an begin iterator");
			}
		#endif

		while ((--it, *this) != begin && !filter.snap.isEnabled(*it)) {
		}

		return *this;
	}

	template<class Snap>
	template<class T>
	auto EntityFilter<Snap>::Iterator<T>::operator++(int) -> Iterator {
		auto temp = *this;
		++*this;
		return temp;
	}

	template<class Snap>
	template<class T>
	auto EntityFilter<Snap>::Iterator<T>::operator--(int) -> Iterator {
		auto temp = *this;
		--*this;
		return temp;
	}
	
	template<class Snap>
	EntityFilter<Snap>::EntityFilter(const Snap& snap, const ComponentBitset cbits)
		: snap{snap}
		, componentsBits{cbits} {
	}
	
	template<class Snap>
	void EntityFilter<Snap>::add(Entity ent, const ComponentBitset& cbits) {
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
	
	template<class Snap>
	void EntityFilter<Snap>::remove(Entity ent) {
		auto pos = std::lower_bound(entities.cbegin(), entities.cend(), ent);

		if (pos != entities.cend() && *pos == ent) {
			entities.erase(pos);
		}
	}
	
	template<class Snap>
	std::size_t EntityFilter<Snap>::size() const {
		return std::distance(begin(), end());
	}
	
	template<class Snap>
	bool EntityFilter<Snap>::empty() const {
		return size() == 0; // TODO: begin != end would be cheaper. Is there a reason we did this? i dont think so.
	}
	
	template<class Snap>
	auto EntityFilter<Snap>::begin() const -> ConstIterator {
		auto it = ConstIterator(*this, entities.begin());

		if (!entities.empty() && !snap.isEnabled(*it)) {
			++it;
		}

		return it;
	}
	
	template<class Snap>
	auto EntityFilter<Snap>::end() const -> ConstIterator {
		return ConstIterator(*this, entities.end());
	}
	
	template<class Snap>
	auto EntityFilter<Snap>::cbegin() const -> ConstIterator {
		return begin();
	}
	
	template<class Snap>
	auto EntityFilter<Snap>::cend() const -> ConstIterator {
		return end();
	}
}
