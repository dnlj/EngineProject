#pragma once

// Engine
#include <Engine/Net/MessageStream.hpp>


namespace Engine::Net {
	void MesssageStream::reset(int32 sz) {
		reset();
		last = curr + sz;
	}

	void MesssageStream::reset() {
		curr = msg.data;
	}

	int32 MesssageStream::size() const {
		return static_cast<int32>(curr - msg.data);
	}

	int32 MesssageStream::capacity() const {
		return sizeof(msg.data);
	}

	char* MesssageStream::data() {
		return msg.data;
	}

	const char* MesssageStream::data() const {
		return msg.data;
	}
}
