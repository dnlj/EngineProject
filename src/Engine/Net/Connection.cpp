#pragma once

// Engine
#include <Engine/Net/Connection.hpp>


namespace Engine::Net {
	Connection::Connection(
		UDPSocket& sock,
		IPv4Address addr,
		Clock::TimePoint lastMessageTime)
		: sock{sock}
		, addr{addr}
		, lastMessageTime{lastMessageTime} {
		// TODO: why not part of writer constructor?
		reset(addr);
	}

	void Connection::next(MessageType type, Channel channel) {
		curr = last;
		write(MessageHeader{
			.type = type,
			.channel = channel,
			.sequence = nextSeqNum[static_cast<int32>(channel)]++,
		});
	}

	MessageHeader& Connection::header() {
		return *reinterpret_cast<MessageHeader*>(curr);
	}

	const MessageHeader& Connection::header() const {
		return *reinterpret_cast<MessageHeader*>(curr);
	}

	char* Connection::data() {
		return packet.data;
	}

	const char* Connection::data() const {
		return packet.data;
	}

	char* Connection::current() {
		return curr;
	}

	const char* Connection::current() const {
		return curr;
	}

	const IPv4Address& Connection::address() const {
		return addr;
	}

	int32 Connection::recv() {
		const int32 len = sock.recv(reinterpret_cast<char*>(&packet), sizeof(packet), addr) - sizeof(packet.header);
		// TODO: filter by PacketHeader.protocol
		reset(len);
		return len;
	}

	int32 Connection::sendto(const IPv4Address& addr) const {
		return sock.send(
			reinterpret_cast<const char*>(&packet),
			static_cast<int32>(last - reinterpret_cast<const char*>(&packet)),
			addr
		);
	}

	int32 Connection::send() {
		const auto sent = sendto(addr);
		reset();
		return sent;
	}

	int32 Connection::flush() {
		if (size() > 0) {
			return send();
		}
		return 0;
	}
	
	void Connection::reset(IPv4Address addr, int32 sz) {
		this->addr = addr;
		reset(sz);
	}

	void Connection::reset(int32 sz) {
		curr = data();
		last = curr + sz;
	}

	void Connection::clear() {
		reset({0,0,0,0, 00000});
	}

	int32 Connection::size() const {
		return static_cast<int32>(last - curr);
	}

	void Connection::write(const std::string& t) {
		write(t.c_str(), t.size() + 1);
	}

	void Connection::read(std::string& t) {
		const auto sz = strlen(curr) + 1;
		ENGINE_DEBUG_ASSERT(curr + sz <= last, "Insufficient space remaining to read");
		t.assign(curr, sz - 1);
		curr += sz;
	}
}
