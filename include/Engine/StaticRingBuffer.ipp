#pragma once

// Engine
#include <Engine/StaticRingBuffer.hpp>


namespace Engine {
	template<class T, uint32 Size>
	StaticRingBuffer<T, Size>::~StaticRingBuffer() {
		clear();
	}

	template<class T, uint32 Size>
	T& StaticRingBuffer<T, Size>::back() noexcept {
		ENGINE_DEBUG_ASSERT(!empty(), "StaticRingBuffer::back called on empty buffer");
		return dataT()[start];
	}

	template<class T, uint32 Size>
	const T& StaticRingBuffer<T, Size>::back() const noexcept {
		return dataT()[start];
	}
	
	template<class T, uint32 Size>
	auto StaticRingBuffer<T, Size>::capacity() const noexcept -> SizeType {
		return Size;
	}

	template<class T, uint32 Size>
	void StaticRingBuffer<T, Size>::clear() {
		while (!empty()) {
			pop();
		}
	}
	
	template<class T, uint32 Size>
	template<class... Args>
	void StaticRingBuffer<T, Size>::emplace(Args&&... args) {
		new (dataT() + stop) T{std::forward<Args>(args)...};
		elementAdded();
	}
	
	template<class T, uint32 Size>
	bool StaticRingBuffer<T, Size>::empty() const noexcept {
		return isEmpty;
	}

	template<class T, uint32 Size>
	bool StaticRingBuffer<T, Size>::full() const noexcept {
		return start == stop && !isEmpty;
	}

	template<class T, uint32 Size>
	void StaticRingBuffer<T, Size>::pop() {
		dataT()[start].~T();
		elementRemoved();
	}

	template<class T, uint32 Size>
	void StaticRingBuffer<T, Size>::push(const T& obj) {
		new (dataT() + stop) T{obj};
		elementAdded();
	}

	template<class T, uint32 Size>
	void StaticRingBuffer<T, Size>::push(T&& obj) {
		new (dataT() + stop) T{std::move(obj)};
		elementAdded();
	}

	template<class T, uint32 Size>
	auto StaticRingBuffer<T, Size>::size() const noexcept -> SizeType {
		return start < stop ? stop - start : stop + Size - start;
	}

	template<class T, uint32 Size>
	T* StaticRingBuffer<T, Size>::dataT() noexcept {
		return reinterpret_cast<T*>(&data);
	}

	template<class T, uint32 Size>
	void StaticRingBuffer<T, Size>::elementAdded() noexcept {
		ENGINE_DEBUG_ASSERT(stop != start || empty(), "Element added to full StaticRingBuffer");
		stop = ++stop % Size;
		isEmpty = false;
	}

	template<class T, uint32 Size>
	void StaticRingBuffer<T, Size>::elementRemoved() noexcept {
		ENGINE_DEBUG_ASSERT(start != stop || full(), "Element removed from empty StaticRingBuffer");
		start = ++start % Size;
		isEmpty = start == stop;
	}
}
