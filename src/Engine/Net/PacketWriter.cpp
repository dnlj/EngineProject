// Engine
#include <Engine/Net/Net.hpp>
#include <Engine/Net/PacketWriter.hpp>


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

	void PacketWriter::updateSentAcks(Channel ch, SequenceNumber nextAck, uint64 acks) {
		auto& ackData = channelAckData[static_cast<int32>(ch)];
		if (nextAck < ackData.nextAck) { return; }

		while (ackData.nextAck < nextAck) {
			const auto i = seqToIndex(ackData.nextAck);
			auto& msg = ackData.messages[i];
			msg.clear();
			//msg.shrink_to_fit();
			//ackData.acks &= ~(1ull << i); // we dont need this since we assign below
			++ackData.nextAck;
		}
		ackData.acks = acks;
	}

	
	int32 PacketWriter::sendAsIs() {
		const auto sent = sock.send(
			reinterpret_cast<const char*>(&packet),
			static_cast<int32>(last - reinterpret_cast<const char*>(&packet)),
			addr
		);
		bytesWritten += sent;
		return sent;
	}

	int32 PacketWriter::send() {
		endMessage();
		return sendAsIs();
	}

	int32 PacketWriter::flush() {
		if (size() > 0) {
			const auto sent = send();
			reset();
			return sent;
		}
		return 0;
	}

	void PacketWriter::reset(int32 sz) {
		curr = packet.data;
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
			sendAsIs();
			memcpy(packet.data, curr, msgsz);
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
