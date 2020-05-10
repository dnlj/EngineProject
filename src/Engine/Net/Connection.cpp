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

	bool Connection::next(MessageType type, Channel channel) {
		if (!canUseChannel(channel)) { return false; }

		store();
		curr = last;
		write(MessageHeader{
			.type = type,
			.channel = channel,
			.sequence = ++(lastSeq[static_cast<int32>(channel)]),
		});

		std::cout << "WRITE: " << header().sequence << " " << this << "\n";

		return true;
	}

	void Connection::ack(const MessageHeader& hdr) {
		switch(hdr.channel) {
			case Channel::RELIABLE: {
				const auto i = hdr.sequence % MAX_UNACKED_MESSAGES;
				reliableData.acks |= 1ull << i;
				while (reliableData.acks & 1) {
					reliableData.acks = reliableData.acks >> 1;
					++reliableData.lastAck;
				}
				std::cout << "ACK: " << hdr.sequence << "\n";
				return;
			}
			case Channel::ORDERED: {
				// TODO: impl
				ENGINE_ERROR("TODO: Unimplemented");
				return;
			}
			case Channel::UNRELIABLE: { return; }
		}
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
	
	bool Connection::canUseChannel(Channel ch) const {
		switch(ch) {
			case Channel::RELIABLE: {
				return lastSeq[static_cast<int32>(Channel::RELIABLE)] - reliableData.lastAck < MAX_UNACKED_MESSAGES;
			}
			case Channel::ORDERED: {
				return lastSeq[static_cast<int32>(Channel::ORDERED)] - orderedData.lastAck < MAX_UNACKED_MESSAGES;
			}
			case Channel::UNRELIABLE: { return true; }
		}

		ENGINE_DEBUG_ASSERT(false, "Unhandled network channel type.");
		return false;
	}

	void Connection::store() {
		const auto sz = size();
		if (sz == 0) { return; }
		const auto& hdr = header();
		const auto i = hdr.sequence % MAX_UNACKED_MESSAGES;
		switch(hdr.channel) {
			case Channel::RELIABLE: {
				auto& msg = reliableData.messages[i];
				ENGINE_DEBUG_ASSERT(!msg);
				msg.reset(new char[sz]);
				memcpy(msg.get(), curr, sz);
				return;
			}
			case Channel::ORDERED: {
				auto& msg = orderedData.messages[i];
				ENGINE_DEBUG_ASSERT(!msg);
				msg.reset(new char[sz]);
				memcpy(msg.get(), curr, sz);
				return;
			}
			case Channel::UNRELIABLE: { return; }
		}
	}
}
