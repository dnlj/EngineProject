#pragma once

// Engine
#include <Engine/Net/MessageStream.hpp>


namespace Engine::Net {
	void MessageStream::reset(int32 sz) {
		reset();
		last = curr + sz;
	}

	void MessageStream::reset() {
		curr = msg.data;
	}

	int32 MessageStream::size() const {
		return static_cast<int32>(curr - msg.data);
	}

	int32 MessageStream::capacity() const {
		return sizeof(msg.data);
	}

	char* MessageStream::data() {
		return msg.data;
	}

	const char* MessageStream::data() const {
		return msg.data;
	}

	
	void MessageStream::write(const std::string& t) {
		write(t.c_str(), t.size() + 1);
	}

	void MessageStream::read(std::string& t) {
		const auto sz = strlen(curr) + 1;
		ENGINE_DEBUG_ASSERT(curr + sz <= last, "Insufficient space remaining to read");
		t.assign(curr, sz - 1);
		curr += sz;
	}
}
