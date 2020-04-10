#pragma once

// Engine
#include <Engine/Net/MessageStream.hpp>


namespace Engine::Net {
	template<class T>
	MessageStream& MessageStream::operator<<(const T& t) {
		write(t);
		return *this;
	}

	template<class T>
	MessageStream& MessageStream::operator>>(T& t) {
		read(t);
		return *this;
	}

	template<class T>
	void MessageStream::write(const T* t, size_t sz) {
		if (last + sz < packet.data + sizeof(packet.data)) {
			memcpy(last, t, sz);
			last += sz;
		} else {
			const auto msgsz = size();
			last = curr;
			send();
			memcpy(data(), curr, msgsz);
			reset(msgsz);
			write(t, sz);
		}

		ENGINE_DEBUG_ASSERT(size() <= MAX_MESSAGE_SIZE, "Message data exceeds MAX_MESSAGE_SIZE = ", MAX_MESSAGE_SIZE, " bytes.");
	};

	template<class T>
	void MessageStream::write(const T& t) {
		write(&t, sizeof(T));
	};

	template<class T, size_t N>
	void MessageStream::write(const T(&t)[N]) {
		write(t, N * sizeof(T));
	}

	template<class T, size_t N>
	void MessageStream::write(const std::array<T, N>& t) {
		write(t.data(), N * sizeof(T));
	}

	template<class T>
	void MessageStream::read(T* t, size_t sz) {
		ENGINE_DEBUG_ASSERT(curr + sz <= last, "Insufficient space remaining to read");
		memcpy(t, curr, sz);
		curr += sz;
	}

	template<class T>
	void MessageStream::read(T& t) {
		read(&t, sizeof(T));
	}

	template<class T, size_t N>
	void MessageStream::read(T(&t)[N]) {
		read(t, N * sizeof(T));
	}

	template<class T, size_t N>
	void MessageStream::read(std::array<T, N>& t) {
		read(t.data(), N * sizeof(T));
	}

	template<class T>
	auto MessageStream::read() {
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
	inline auto MessageStream::read<char[]>() {
		return read<std::string>();
	}
}
