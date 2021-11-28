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
