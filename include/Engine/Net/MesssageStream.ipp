#pragma once

// Engine
#include <Engine/Net/MessageStream.hpp>


namespace Engine::Net {
	template<class T>
	MesssageStream& MesssageStream::operator<<(const T& t) {
		write(t);
		return *this;
	}

	template<class T>
	MesssageStream& MesssageStream::operator>>(T& t) {
		read(t);
		return *this;
	}

	template<class T>
	void MesssageStream::write(const T& t) {
		constexpr auto sz = sizeof(T);
		ENGINE_DEBUG_ASSERT(curr + sz < msg.data + sizeof(msg.data), "Insufficient space remaining to write");
		memcpy(curr, &t, sz);
		curr += sz;
		last += sz;
	};

	template<class T>
	void MesssageStream::read(T* t, size_t sz) {
		ENGINE_DEBUG_ASSERT(curr + sz <= last, "Insufficient space remaining to read");
		memcpy(t, curr, sz);
		curr += sz;
	}

	template<class T>
	void MesssageStream::read(T& t) {
		read(&t, sizeof(T));
	}

	template<class T, size_t N>
	void MesssageStream::read(T(&t)[N]) {
		read(t, N * sizeof(T));
	}

	template<class T, size_t N>
	void MesssageStream::read(std::array<T, N>& t) {
		read(t.data(), N * sizeof(T));
	}

	template<class T>
	auto MesssageStream::read() {
		if constexpr (std::is_bounded_array_v<T> && std::rank_v<T> == 1) {
			// TODO: N dimension
			std::array<std::remove_extent_t<T>, std::extent_v<T>> t;
			read(t.data(), sizeof(T));
			return t;
		} else {
			T t;
			read(t);
			return t;
		}
	}

	template<>
	inline auto MesssageStream::read<char[]>() {
		const auto sz = strlen(curr) + 1;
		ENGINE_DEBUG_ASSERT(curr + sz <= last, "Insufficient space remaining to read");
		std::string t{curr, sz - 1};
		curr += sz;
		return t;
	}
}
