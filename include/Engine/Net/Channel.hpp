#pragma once

// Engine
#include <Engine/Net/MessageHeader.hpp>
#include <Engine/Net/PacketWriter.hpp>


namespace Engine::Net::Detail {
	template<MessageType... Ms>
	class Channel_Base { // TODO: name?
		// TODO: static_assert that all messages ids are continuous
		public:
			Channel_Base() = default;
			Channel_Base(const Channel_Base&) = delete;

			template<auto M>
			constexpr static bool handlesMessageType() {
				return ((M == Ms) || ...);
			}

			//constexpr static bool canWriteMessage() noexcept {
			//	// TODO: static assert error if called
			//	return false;
			//}
			//
			//bool msgBegin(SeqNum pktSeq, MessageHeader& hdr) {
			//}
			//
			//void msgEnd(SeqNum pktSeq, MessageHeader& hdr) {
			//	hdr.seq = ++nextSeq;
			//}
			//
			//void recvPacketAck(SeqNum seq) {
			//}
			//
			//void resend(PacketNode* node) {
			//}
	};
}

namespace Engine::Net {
	template<MessageType... Ms>
	class Channel_UnreliableUnordered : public Detail::Channel_Base<Ms...> { // TODO: name?
		private:
			SeqNum nextSeq = -1;

		public:
			constexpr static bool canWriteMessage() noexcept {
				return true;
			}

			bool msgBegin(SeqNum pktSeq, MessageHeader& hdr) {
			}

			void msgEnd(SeqNum pktSeq, MessageHeader& hdr) {
				hdr.seq = ++nextSeq;
			}

			void recvPacketAck(SeqNum seq) {
			}

			void resend(PacketNode* node) {
			}
	};

	template<MessageType... Ms>
	class Channel_ReliableUnordered : public Detail::Channel_Base<Ms...> { // TODO: name?
		private:
			struct MsgData {
				bool acked = true;
				std::vector<byte> data;
			};

			SeqNum minAck = -1;
			SeqNum nextSeq = 0;
			MsgData msgData[64] = {};

			constexpr static auto index(SeqNum seq) noexcept { return seq % std::extent_v<decltype(msgData)>; }
			auto& get(SeqNum seq) { return msgData[index(seq)]; }
			const auto& get(SeqNum seq) const { return const_cast<Channel2*>(this)->get(seq); }

		public:
			bool canWriteMessage() const {
				return get(nextSeq).acked;
			}

			// TODO: do we even need this?
			bool msgBegin(SeqNum pktSeq, MessageHeader& hdr) {
			}

			void msgEnd(SeqNum pktSeq, MessageHeader& hdr) {
				hdr.seq = nextSeq++;
				auto& data = get(hdr.seq);
				data.acked = false;
				data.assign(&hdr, &hdr + hdr->size);
			}

			void recvPacketAck(SeqNum seq) {
			}

			// TODO: want some kind of fill-rest-of-packet function also
			void writeUnacked(PacketWriter& packetWriter) {
				for (auto i = minAck; i < nextSeq; ++i) {
					auto& data = get(i);
					// TODO: this wont work since we havent called conn.msgBegin...
					// TODO: fix names...
					packetWriter.write(data.data.data(), data.data.size());
				}
			}
	};
}
