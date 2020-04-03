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

	
	void MesssageStream::write(const std::string& t) {
		write(t.c_str(), t.size() + 1);
	}

	void MesssageStream::read(std::string& t) {
		const auto sz = strlen(curr) + 1;
		ENGINE_DEBUG_ASSERT(curr + sz <= last, "Insufficient space remaining to read");
		t.assign(curr, sz - 1);
		curr += sz;
	}
}
