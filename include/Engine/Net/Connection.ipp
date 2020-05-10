#pragma once

// Engine
#include <Engine/Net/Connection.hpp>


namespace Engine::Net {
	template<class T>
	Connection& Connection::operator<<(const T& t) {
		write(t);
		return *this;
	}

	template<class T>
	Connection& Connection::operator>>(T& t) {
		read(t);
		return *this;
	}

	constexpr int32 Connection::capacity() {
		return sizeof(packet.data);
	}

	template<class T>
	void Connection::write(const T* t, size_t sz) {
		if (last + sz <= packet.data + sizeof(packet.data)) {
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

		ENGINE_DEBUG_ASSERT(sz <= MAX_MESSAGE_SIZE, "Message data exceeds MAX_MESSAGE_SIZE = ", MAX_MESSAGE_SIZE, " bytes.");
	};

	template<class T>
	void Connection::write(const T& t) {
		write(&t, sizeof(T));
	};

	template<class T, size_t N>
	void Connection::write(const T(&t)[N]) {
		write(t, N * sizeof(T));
	}

	template<class T, size_t N>
	void Connection::write(const std::array<T, N>& t) {
		write(t.data(), N * sizeof(T));
	}

	template<class T>
	void Connection::read(T* t, size_t sz) {
		ENGINE_DEBUG_ASSERT(curr + sz <= last, "Insufficient space remaining to read");
		memcpy(t, curr, sz);
		curr += sz;
	}

	template<class T>
	void Connection::read(T& t) {
		read(&t, sizeof(T));
	}

	template<class T, size_t N>
	void Connection::read(T(&t)[N]) {
		read(t, N * sizeof(T));
	}

	template<class T, size_t N>
	void Connection::read(std::array<T, N>& t) {
		read(t.data(), N * sizeof(T));
	}

	template<class T>
	auto Connection::read() {
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
	inline auto Connection::read<char[]>() {
		return read<std::string>();
	}
}
