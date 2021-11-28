#pragma once

// Engine
#include <Engine/StaticRingBuffer.hpp>


namespace Engine::detail {
	template<class T, uint32 Size>
	auto RingBufferImpl<T, Size>::operator=(const RingBufferImpl& other) -> RingBufferImpl& {
		clear();

		if constexpr (!IsStatic) { // TODO: replace with reserve(other.size());
			data.second = other.data.second;
			data.first = new char[sizeof(T) * data.second];
		}

		for (const auto& v : other) {
			push(v);
		}

		return *this;
	}

	template<class T, uint32 Size>
	template<class... Args>
	void RingBufferImpl<T, Size>::emplace(Args&&... args) {
		ensureSpace();
		new (dataT() + stop) T{std::forward<Args>(args)...};
		elementAdded();
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
			const auto sz = data.second + (data.second / 2); // 1.5 Growth factor
			RingBufferImpl<T, Size> other{sz};

			while (!empty()) {
				other.push(std::move(front()));
				pop();
			}

			*this = std::move(other);
		}
	}
}
