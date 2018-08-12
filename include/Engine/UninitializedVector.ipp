#pragma once

// STD
#include <new>


namespace Engine {
	template<class T>
	UninitializedVector<T>::~UninitializedVector() {
		::operator delete(data);
	}

	template<class T>
	std::size_t UninitializedVector<T>::size() const {
		return count;
	}
}
