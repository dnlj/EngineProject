#pragma once


namespace Engine {
	template<class T>
	std::size_t UninitializedVector<T>::size() const {
		return count;
	}
}
