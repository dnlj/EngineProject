#pragma once

// Engine
#include <Engine/Net/MessageStream.hpp>


namespace Engine::Net {
	MessageStream::MessageStream(UDPSocket& socket)
		: sock{socket} {
	}

	void MessageStream::next(MessageType type, MessageChannel channel) {
		curr = last;
		write(MessageHeader{
			.type = type,
			.channel = channel,
			.sequence = nextSeqNum[channel]++,
		});
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

	char* MessageStream::current() {
		return curr;
	}

	const char* MessageStream::current() const {
		return curr;
	}

	const IPv4Address& MessageStream::address() const {
		return addr;
	}

	int32 MessageStream::recv() {
		const int32 len = sock.recv(reinterpret_cast<char*>(&packet), sizeof(packet), addr) - sizeof(packet.header);
		// TODO: filter by PacketHeader.protocol
		reset(len);
		return len;
	}

	int32 MessageStream::sendto(const IPv4Address& addr) const {
		return sock.send(
			reinterpret_cast<const char*>(&packet),
			static_cast<int32>(last - reinterpret_cast<const char*>(&packet)),
			addr
		);
	}

	int32 MessageStream::send() {
		const auto sent = sendto(addr);
		reset();
		return sent;
	}

	int32 MessageStream::flush() {
		if (size() > 0) {
			return send();
		}
		return 0;
	}
	
	void MessageStream::reset(IPv4Address addr, int32 sz) {
		this->addr = addr;
		reset(sz);
	}

	void MessageStream::reset(int32 sz) {
		curr = data();
		last = curr + sz;
	}

	void MessageStream::clear() {
		reset({0,0,0,0, 00000});
	}

	int32 MessageStream::size() const {
		return static_cast<int32>(last - curr);
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
