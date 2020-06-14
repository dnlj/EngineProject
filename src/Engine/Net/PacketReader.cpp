// Engine
#include <Engine/Net/PacketReader.hpp>
#include <Engine/Net/Net.hpp>


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

		const auto i = seqToIndex(seq);
		auto bit = 1ull << i;
		if (ackData.acks & bit) { return false; }

		if (hdr.channel == Channel::RELIABLE) {
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
		bytesRead += sz;
		return temp;
	}

	bool PacketReader::next() {
		if (size() != 0) { return true; }

		// Ordered messages
		auto& ackData = channelAckData[static_cast<int32>(Channel::ORDERED)];
		const auto i = seqToIndex(ackData.nextAck);
		const auto bit = 1ull << i;

		if (ackData.acks & bit) {
			auto& msg = ackData.messages[i];
			curr = msg.data();
			last = curr + msg.size();
			return true;
		}

		return false;
	}

	int32 PacketReader::size() const {
		return static_cast<int32>(last - curr);
	}

	const AckData& PacketReader::getAckData(Channel ch) const {
		return channelAckData[static_cast<int32>(ch)];
	}
}
