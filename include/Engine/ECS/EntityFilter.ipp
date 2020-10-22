#pragma once


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

		while ((++it, *this) != end && !filter.isEnabled(*it)) {
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

		while ((--it, *this) != begin && !filter.isEnabled(*it)) {
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
