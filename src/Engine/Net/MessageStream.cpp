#pragma once

// Engine
#include <Engine/Net/MessageStream.hpp>


namespace Engine::Net {
	MessageStream::MessageStream() {
	}

	void MessageStream::setSocket(UDPSocket& sock) {
		this->sock = &sock;
	}

	void MessageStream::setAddress(IPv4Address& addr) {
		this->addr = &addr;
	}

	void MessageStream::next() {
		curr = last;
		last += sizeof(MessageHeader);
	}

	MessageHeader& MessageStream::header() {
		return *reinterpret_cast<MessageHeader*>(curr);
	}

	const MessageHeader& MessageStream::header() const {
		return *reinterpret_cast<MessageHeader*>(curr);
	}

	char* MessageStream::data() {
		return packet.data;
	}

	const char* MessageStream::data() const {
		return packet.data;
	}

	int32 MessageStream::recv() {
		const auto len = sock->recv(reinterpret_cast<char*>(&packet), sizeof(packet), *addr);
		reset(len);
		return len;
	}

	int32 MessageStream::send() const {
		return sock->send(
			reinterpret_cast<const char*>(&packet),
			static_cast<int32>(last - reinterpret_cast<const char*>(&packet)),
			*addr
		);
	}

	void MessageStream::reset(int32 sz) {
		curr = data();
		last = curr + sz;
	}

	int32 MessageStream::size() const {
		return static_cast<int32>(last - curr);
	}

	int32 MessageStream::capacity() const {
		return sizeof(packet.data);
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
