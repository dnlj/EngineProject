#pragma once

// Engine
#include <Engine/StaticRingBuffer.hpp>


namespace Engine::detail {
	template<class T, uint32 Size>
	auto RingBufferImpl<T, Size>::operator=(const RingBufferImpl& other) -> RingBufferImpl& {
		clear();
		reserve(other.data.second);

		for (const auto& v : other) {
			push(v);
		}

		return *this;
	}

	template<class T, uint32 Size>
	void RingBufferImpl<T, Size>::ensureSpace() {
		if constexpr (!IsStatic) {
			if (!full()) { return; }
			reserve(data.second + (data.second / 2));
		}
	}
}
