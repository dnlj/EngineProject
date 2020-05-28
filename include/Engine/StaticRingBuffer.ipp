#pragma once

// Engine
#include <Engine/StaticRingBuffer.hpp>


namespace Engine::detail {
	template<class T, uint32 Size>
	template<class>
	RingBufferImpl<T, Size>::RingBufferImpl() {
	}

	template<class T, uint32 Size>
	template<class>
	RingBufferImpl<T, Size>::RingBufferImpl(SizeType sz) {
		data.second = sz;
		data.first = new char[sizeof(T) * sz];
	}

	template<class T, uint32 Size>
	RingBufferImpl<T, Size>::~RingBufferImpl() {
		clear();

		if constexpr (!IsStatic) {
			delete[] data.first;
		}
	}

	template<class T, uint32 Size>
	T& RingBufferImpl<T, Size>::operator[](SizeType i) {
		return dataT()[wrapIndex(start + i)];
	}

	template<class T, uint32 Size>
	const T& RingBufferImpl<T, Size>::operator[](SizeType i) const {
		return reinterpret_cast<RingBufferImpl&>(*this)[i];
	}

	template<class T, uint32 Size>
	T& RingBufferImpl<T, Size>::back() noexcept {
		ENGINE_DEBUG_ASSERT(!empty(), "RingBufferImpl::back called on empty buffer");
		return dataT()[start];
	}

	template<class T, uint32 Size>
	const T& RingBufferImpl<T, Size>::back() const noexcept {
		return reinterpret_cast<RingBufferImpl&>(*this).back();
	}

	template<class T, uint32 Size>
	auto RingBufferImpl<T, Size>::begin() noexcept -> Iterator {
		return {*this, start};
	}

	template<class T, uint32 Size>
	auto RingBufferImpl<T, Size>::begin() const noexcept -> ConstIterator {
		return cbegin();
	}

	template<class T, uint32 Size>
	auto RingBufferImpl<T, Size>::cbegin() const noexcept -> ConstIterator {
		return {*this, start};
	}
	
	template<class T, uint32 Size>
	auto RingBufferImpl<T, Size>::capacity() const noexcept -> SizeType {
		if constexpr (IsStatic) {
			return Size;
		} else {
			return data.second;
		}
	}

	template<class T, uint32 Size>
	void RingBufferImpl<T, Size>::clear() {
		while (!empty()) {
			pop();
		}
	}
	
	template<class T, uint32 Size>
	template<class... Args>
	void RingBufferImpl<T, Size>::emplace(Args&&... args) {
		ensureSpace();
		new (dataT() + stop) T{std::forward<Args>(args)...};
		elementAdded();
	}
	
	template<class T, uint32 Size>
	bool RingBufferImpl<T, Size>::empty() const noexcept {
		return isEmpty;
	}

	template<class T, uint32 Size>
	auto RingBufferImpl<T, Size>::end() noexcept -> Iterator {
		return {*this, stop};
	}

	template<class T, uint32 Size>
	auto RingBufferImpl<T, Size>::end() const noexcept -> ConstIterator {
		return cend();
	}

	template<class T, uint32 Size>
	auto RingBufferImpl<T, Size>::cend() const noexcept -> ConstIterator {
		return {*this, stop};
	}

	template<class T, uint32 Size>
	T& RingBufferImpl<T, Size>::front() noexcept {
		ENGINE_DEBUG_ASSERT(!empty(), "RingBufferImpl::front called on empty buffer");
		return dataT()[wrapIndex(stop - 1)];
	}

	template<class T, uint32 Size>
	const T& RingBufferImpl<T, Size>::front() const noexcept {
		return reinterpret_cast<RingBufferImpl&>(*this).front();
	}

	template<class T, uint32 Size>
	bool RingBufferImpl<T, Size>::full() const noexcept {
		return start == stop && !isEmpty;
	}

	template<class T, uint32 Size>
	 void RingBufferImpl<T, Size>::pop() {
		dataT()[start].~T();
		elementRemoved();
	}

	template<class T, uint32 Size>
	void RingBufferImpl<T, Size>::push(const T& obj) {
		ensureSpace();
		new (dataT() + stop) T{obj};
		elementAdded();
	}

	template<class T, uint32 Size>
	void RingBufferImpl<T, Size>::push(T&& obj) {
		ensureSpace();
		new (dataT() + stop) T{std::move(obj)};
		elementAdded();
	}

	template<class T, uint32 Size>
	auto RingBufferImpl<T, Size>::size() const noexcept -> SizeType {
		return stop < start ? stop + capacity() - start : stop - start;
	}
	
	template<class T, uint32 Size>
	void RingBufferImpl<T, Size>::swap(RingBufferImpl& other) {
		using std::swap;
		swap(data, other.data);
		swap(start, other.start);
		swap(stop, other.stop);
		swap(isEmpty, other.isEmpty);
	}

	template<class T, uint32 Size>
	T* RingBufferImpl<T, Size>::dataT() noexcept {
		if constexpr (IsStatic) {
			return reinterpret_cast<T*>(&data);
		} else {
			return reinterpret_cast<T*>(data.first);
		}
	}

	template<class T, uint32 Size>
	void RingBufferImpl<T, Size>::elementAdded() noexcept {
		ENGINE_DEBUG_ASSERT(!full(), "Element added to full RingBuffer");
		stop = wrapIndex(++stop);
		isEmpty = false;
	}

	template<class T, uint32 Size>
	void RingBufferImpl<T, Size>::elementRemoved() noexcept {
		ENGINE_DEBUG_ASSERT(!empty(), "Element removed from empty RingBuffer");
		start = wrapIndex(++start);
		isEmpty = start == stop;
	}
	
	template<class T, uint32 Size>
	void RingBufferImpl<T, Size>::ensureSpace() {
		if constexpr (!IsStatic) {
			if (!full()) { return; }
			const auto sz = data.second * 2; // TODO: look into ideal growth factor
			RingBufferImpl<T, Size> other{sz};

			while (!empty()) {
				other.push(std::move(back()));
				pop();
			}

			swap(other);
		}
	}
	template<class T, uint32 Size>
	auto RingBufferImpl<T, Size>::wrapIndex(SizeType i) -> SizeType {
		const auto c = capacity();
		return (i + c) % c;
	}
}
