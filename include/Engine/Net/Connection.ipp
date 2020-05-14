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
		// TODO: impl. current impl is wrong.
		assert(false);
		read(t);
		return *this;
	}

	constexpr int32 Connection::capacity() {
		return sizeof(packet.data);
	}

	template<class T>
	void Connection::write(const T& t) {
		write(&t, sizeof(T));
	};

	template<class T>
	auto Connection::read() {
		if constexpr (std::is_same_v<std::decay_t<T>, char*>) {
			return reinterpret_cast<const char*>(read(strlen(curr) + 1));
		} else {
			return *reinterpret_cast<const T*>(read(sizeof(T)));
		}
	}
}
