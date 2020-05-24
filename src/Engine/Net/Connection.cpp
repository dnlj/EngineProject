#pragma once

// Engine
#include <Engine/Net/Connection.hpp>
#include <Engine/Utility/Utility.hpp>


//////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: move
namespace Engine::Net {
	PacketWriter::PacketWriter(UDPSocket& sock, IPv4Address addr) : sock{sock}, addr{addr} {
		reset();
	}

	MessageHeader& PacketWriter::header() {
		return *reinterpret_cast<MessageHeader*>(curr);
	}

	const MessageHeader& PacketWriter::header() const {
		return *reinterpret_cast<MessageHeader*>(curr);
	}

	char* PacketWriter::current() {
		return curr;
	}

	/** @copydoc current */
	const char* PacketWriter::current() const {
		return curr;
	}

	char* PacketWriter::data() {
		return packet.data;
	}

	const char* PacketWriter::data() const {
		return packet.data;
	}

	void PacketWriter::updateSentAcks(Channel ch, SequenceNumber nextAck, uint64 acks) {
		auto& ackData = channelAckData[static_cast<int32>(ch)];
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

	int32 PacketWriter::sendto() {
		endMessage();
		return sock.send(
			reinterpret_cast<const char*>(&packet),
			static_cast<int32>(last - reinterpret_cast<const char*>(&packet)),
			addr
		);
	}

	int32 PacketWriter::send() {
		const auto sent = sendto();
		reset();
		return sent;
	}

	int32 PacketWriter::flush() {
		if (size() > 0) {
			return send();
		}
		return 0;
	}

	void PacketWriter::reset(int32 sz) {
		curr = data();
		last = curr + sz;
	}

	int32 PacketWriter::size() const {
		return static_cast<int32>(last - curr);
	}

	bool PacketWriter::next(MessageType type, Channel channel) {
		if (!canUseChannel(channel)) { return false; }

		endMessage();
		write(MessageHeader{
			.type = type,
			.channel = channel,
			.sequence = nextSeq[static_cast<int32>(channel)]++,
		});

		return true;
	}

	void PacketWriter::write(const void* t, size_t sz) {
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

	void PacketWriter::write(const std::string& t) {
		write(t.c_str(), t.size() + 1);
	}

	void PacketWriter::write(const char* t) {
		write(t, strlen(t) + 1);
	}

	void PacketWriter::store() {
		const auto sz = size();
		if (sz == 0) { return; }

		const auto& hdr = header();
		if (hdr.channel == Channel::UNRELIABLE) { return; }

		auto& ackData = channelAckData[static_cast<int32>(hdr.channel)];
		auto& msg = ackData.messages[seqToIndex(hdr.sequence)];

		if (msg.empty()) {
			msg.assign(curr, last);
		}
	}
	
	void PacketWriter::writeUnacked(Channel ch) {
		const auto& ackData = channelAckData[static_cast<int32>(ch)];
		const auto min = ackData.nextAck;
		const auto max = std::min(nextSeq[static_cast<int32>(ch)], min + MAX_UNACKED_MESSAGES - 1);

		for (auto seq = min; seq <= max; ++seq) {
			const auto i = seqToIndex(seq);
			if ((ackData.acks & (1ull << i)) == 0) {
				const auto& msg = ackData.messages[i];
				if (!msg.empty()) {
					endMessage();
					write(msg.data(), msg.size());
				}
			}
		}
	}

	void PacketWriter::endMessage() {
		header().size = size() - static_cast<uint16>(sizeof(MessageHeader));
		store();
		curr = last;
	}
	
	bool PacketWriter::canUseChannel(Channel ch) const {
		if (ch == Channel::UNRELIABLE) { return true; }
		auto& ackData = channelAckData[static_cast<int32>(ch)];
		return nextSeq[static_cast<int32>(ch)] - ackData.nextAck <= MAX_UNACKED_MESSAGES;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: move
namespace Engine::Net {
	void PacketReader::set(char* curr, char* last) {
		this->curr = curr;
		this->last = last;
	}

	bool PacketReader::updateRecvAcks(const MessageHeader& hdr) {
		const auto seq = hdr.sequence;
		auto& ackData = channelAckData[static_cast<int32>(hdr.channel)];
		const auto min = ackData.nextAck;
		const auto max = min + MAX_UNACKED_MESSAGES - 1;

		if (seq < min || seq > max) { return false; }

		//std::cout << "UPDATE RECV ACK: " << seq << " " << (int)hdr.channel << " " << this << "\n";
		const auto i = seqToIndex(seq);
		auto bit = 1ull << i;

		// TODO: simplify
		if (hdr.channel == Channel::RELIABLE) {
			if (ackData.acks & bit) { return false; }
			ackData.acks |= bit;

			while(ackData.acks & (bit = 1ull << seqToIndex(ackData.nextAck))) {
				ackData.acks &= ~bit;
				++ackData.nextAck;
			}

			return true;
		} else if (seq == ackData.nextAck) {
			ackData.acks &= ~bit;
			++ackData.nextAck;
			return true;
		}

		// Channel::ORDERED and seq != nextAck. Store for later replay.
		std::cout << "OUT OF ORDER " << seq << "\n";
		ackData.acks |= bit;
		const char* start = reinterpret_cast<const char*>(&hdr);
		ackData.messages[i].assign(start, start + sizeof(MessageHeader) + hdr.size);
		return false;
	}

	const void* PacketReader::read(size_t sz) {
		ENGINE_DEBUG_ASSERT(curr + sz <= last, "Insufficient space remaining to read");
		if (curr + sz > last) { return nullptr; }

		const void* temp = curr;
		curr += sz;
		return temp;
	}

	bool PacketReader::next() {
		if (size() != 0) { return true; }

		// Ordered messages
		auto& ackData = channelAckData[static_cast<int32>(Channel::ORDERED)];
		const auto i = seqToIndex(ackData.nextAck);
		const auto bit = 1ull << i;

		if (ackData.acks & bit) {
			// Update curr/last ptrs
			auto& msg = ackData.messages[i];
			curr = msg.data();
			last = curr + msg.size();
			return true;
		}

		return false;
	}
}


namespace Engine::Net {
	Connection::Connection(
		UDPSocket& sock,
		IPv4Address addr,
		Clock::TimePoint lastMessageTime)
		: writer{sock, addr}
		, lastMessageTime{lastMessageTime} {
	}
	
	void Connection::writeRecvAcks(Channel ch) {
		auto& ackData = reader.channelAckData[static_cast<int32>(ch)];
		writer.write(ch);
		writer.write(ackData.nextAck);
		writer.write(ackData.acks);
	}

	const IPv4Address& Connection::address() const {
		return writer.addr;
	}

	constexpr SequenceNumber seqToIndex(SequenceNumber seq) {
		static_assert(Engine::Utility::isPowerOfTwo(MAX_UNACKED_MESSAGES));
		return seq & (MAX_UNACKED_MESSAGES - 1);
	}
}
