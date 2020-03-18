#pragma once

// Engine
#include <Engine/StaticRingBuffer.hpp>


namespace Engine {
	template<class T, uint32 Size>
	StaticRingBuffer<T, Size>::~StaticRingBuffer() {
		clear();
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
		advance(stop);
	}
	
	template<class T, uint32 Size>
	bool StaticRingBuffer<T, Size>::empty() const noexcept {
		return start != stop;
	}

	template<class T, uint32 Size>
	void StaticRingBuffer<T, Size>::pop() {
		ENGINE_ASSERT(!empty(), "Pop called on empty ring buffer");
		dataT()[start].~T();
		next(start);
	}

	template<class T, uint32 Size>
	void StaticRingBuffer<T, Size>::push(const T& obj) {
		new (dataT() + stop) T{obj};
		next(stop);
	}

	template<class T, uint32 Size>
	void StaticRingBuffer<T, Size>::push(T&& obj) {
		new (dataT() + stop) T{std::move(obj)};
		next(stop);
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
	constexpr void StaticRingBuffer<T, Size>::next(SizeType& value) noexcept {
		value = ++value % Size;
	}
}
