#pragma once

// Engine
#include <Engine/Net/Connection.hpp>
#include <Engine/Utility/Utility.hpp>


namespace Engine::Net {
	Connection::Connection(
		UDPSocket& sock,
		IPv4Address addr,
		Clock::TimePoint lastMessageTime)
		: sock{sock}
		, addr{addr}
		, lastMessageTime{lastMessageTime} {
		reset(addr);
	}

	void Connection::endMessage() {
		header().size = size() - static_cast<uint16>(sizeof(MessageHeader));
		store();
		curr = last;
	}

	bool Connection::next(MessageType type, Channel channel) {
		if (!canUseChannel(channel)) { return false; }

		endMessage();
		write(MessageHeader{
			.type = type,
			.channel = channel,
			.sequence = nextSeq[static_cast<int32>(channel)]++,
		});

		return true;
	}

	void Connection::updateSentAcks(Channel ch, SequenceNumber nextAck, uint64 acks) {
		auto& ackData = sentAckData[static_cast<int32>(ch)];
		if (nextAck < ackData.nextAck) { return; }

		while (ackData.nextAck < nextAck) {
			const auto i = seqToIndex(ackData.nextAck);
			auto& msg = ackData.messages[i];
			msg.clear();
			//msg.shrink_to_fit();
			//ackData.acks &= ~(1ull << i); // we dont need this since we assign below
			std::cout << "UPDATE SENT ACK: " << ackData.nextAck << " " << (int)ch << " " << this << "\n";
			++ackData.nextAck;
		}
		ackData.acks = acks;
	}

	bool Connection::updateRecvAcks(const MessageHeader& hdr) {
		const auto seq = hdr.sequence;
		auto& ackData = recvAckData[static_cast<int32>(hdr.channel)];
		const auto min = ackData.nextAck;
		const auto max = min + MAX_UNACKED_MESSAGES - 1;

		if (seq < min || seq > max) { return false; }

		ackData.acks |= 1ull << seqToIndex(seq);

		for (decltype(ackData.acks) curr;
			(curr = 1ull << seqToIndex(ackData.nextAck)), (ackData.acks & curr);
			++ackData.nextAck) {
			ackData.acks &= ~curr;
			std::cout << "UPDATE RECV ACK: " << ackData.nextAck << " " << (int)hdr.channel << " " << this << "\n";
		}

		return true;
	}
	
	void Connection::writeRecvAcks(Channel ch) {
		auto& ackData = recvAckData[static_cast<int32>(ch)];
		write(ch);
		write(ackData.nextAck);
		write(ackData.acks);
	}

	void Connection::writeUnacked(Channel ch) {
		const auto& ackData = sentAckData[static_cast<int32>(ch)];
		const auto min = ackData.nextAck;
		const auto max = std::min(nextSeq[static_cast<int32>(ch)], min + MAX_UNACKED_MESSAGES - 1);

		for (auto seq = min; seq < max; ++seq) {
			const auto i = seqToIndex(seq);
			if ((ackData.acks & (1ull << i)) == 0) {
				const auto& msg = ackData.messages[i];
				if (!msg.empty()) {
					endMessage();
					write(msg.data(), msg.size());

					std::cout << "Send " << seq << " " << (int)ch << "\n";
				}
			}
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

	int32 Connection::sendto(const IPv4Address& addr) {
		endMessage();
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

	void Connection::write(const void* t, size_t sz) {
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

	void Connection::write(const std::string& t) {
		write(t.c_str(), t.size() + 1);
	}

	void Connection::write(const char* t) {
		write(t, strlen(t) + 1);
	}

	const void* Connection::read(size_t sz) {
		ENGINE_DEBUG_ASSERT(curr + sz <= last, "Insufficient space remaining to read");
		if (curr + sz > last) { return nullptr; }

		const void* temp = curr;
		curr += sz;
		return temp;
	}
	
	bool Connection::canUseChannel(Channel ch) const {
		if (ch == Channel::UNRELIABLE) { return true; }
		auto& ackData = sentAckData[static_cast<int32>(ch)];
		return nextSeq[static_cast<int32>(ch)] - ackData.nextAck <= MAX_UNACKED_MESSAGES;
	}

	void Connection::store() {
		const auto sz = size();
		if (sz == 0) { return; }

		const auto& hdr = header();
		if (hdr.channel == Channel::UNRELIABLE) { return; }

		auto& ackData = sentAckData[static_cast<int32>(hdr.channel)];
		auto& msg = ackData.messages[seqToIndex(hdr.sequence)];

		if (msg.empty()) {
			msg.assign(curr, last);
		}
	}

	constexpr SequenceNumber Connection::seqToIndex(SequenceNumber seq) {
		static_assert(Engine::Utility::isPowerOfTwo(MAX_UNACKED_MESSAGES));
		return seq & (MAX_UNACKED_MESSAGES - 1);
	}
}
