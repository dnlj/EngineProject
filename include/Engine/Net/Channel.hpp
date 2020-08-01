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

			void recvPacketAck(SeqNum seq) {}
			void writeUnacked(PacketWriter& packetWriter) {}

	};
}

namespace Engine::Net {
	template<class I> // TODO: move
	ENGINE_INLINE constexpr bool seqGreater(I a, I b) noexcept {
		constexpr auto half = std::numeric_limits<I>::max() / 2 + 1;
		return ((a > b) && (a - b <= half))
			|| ((a < b) && (b - a >  half));
	}

	template<class I> // TODO: move
	ENGINE_INLINE constexpr bool seqLess(I a, I b) noexcept {
		return seqGreater<I>(b, a);
	}

	// TODO: move
	// TODO: doc
	template<class S, class T, int32 N> // TODO: Doc destructive to old entities as new are inserted
	class SeqBuffer { // Almost SparseSet + RingBuffer. Not quite.
		private:
			struct Entry {
				S seq;
				bool valid = false;
			};

			S lowest = 0;
			S next = 0;
			Entry entries[N] = {};
			T storage[N];

			constexpr static auto index(S seq) noexcept { return seq % N; }

			ENGINE_INLINE auto& getEntry(S seq) { return entries[index(seq)]; }
			ENGINE_INLINE const auto& getEntry(S seq) const { return const_cast<SeqBuffer*>(this)->getEntry(seq); }

		public:
			constexpr static auto capacity() noexcept { return N; }

			auto min() const { return lowest; }
			auto max() const { return next - 1; }

			void clear() {
				memset(&entries, 0, sizeof(entries));
			}

			bool spaceFor(S seq) const {
				return !getEntry(seq).valid;
			}
			
			T& insert(S seq) {
				// TODO: seq < min check

				if (seqGreater(seq + 1, next)) { next = seq + 1; }

				// TODO: this could also just be done with one or two memsets
				while (seqLess(lowest + capacity() - 1, seq)) {
					// TODO: BUG: currently we call remove even if no entry exists atm
					remove(lowest);
				}

				getEntry(seq) = {
					.seq = seq,
					.valid = true,
				};

				auto& data = get(seq);
				data = {}; // TODO: do we want this?
				return data;
			}

			void remove(S seq) {
				getEntry(seq).valid = false;

				while (seqLess(lowest, next) && !getEntry(lowest).valid) {
					++lowest;
				}
			}

			T* find(S seq) {
				const auto i = index(seq);
				const auto& e = entries[i];
				return (e.valid && (e.seq == seq)) ? &storage[i] : nullptr;
			}

			ENGINE_INLINE T& get(S seq) {
				return storage[index(seq)];
			}

			ENGINE_INLINE const T& get(S seq) const {
				return const_cast<SeqBuffer*>(this)->get(seq);
			}
	};

	template<MessageType... Ms>
	class Channel_UnreliableUnordered : public Detail::Channel_Base<Ms...> { // TODO: name?
		private:
			SeqNum nextSeq = -1;

		public:
			constexpr static bool canWriteMessage() noexcept {
				return true;
			}

			void msgEnd(SeqNum pktSeq, MessageHeader& hdr) {
				hdr.seq = ++nextSeq;
			}
	};

	template<MessageType... Ms>
	class Channel_ReliableUnordered : public Detail::Channel_Base<Ms...> { // TODO: name?
		private:
			/** The sequence number to use for the next message */
			SeqNum nextSeq = 0;

			struct MsgData {
				std::vector<byte> data;
			};
			SeqBuffer<SeqNum, MsgData, 64> msgData;

			struct PacketData {
				std::vector<SeqNum> messages;
			};
			SeqBuffer<SeqNum, PacketData, 64> pktData;

		public:
			bool canWriteMessage() const {
				return msgData.spaceFor(nextSeq);
			}

			void msgEnd(SeqNum pktSeq, MessageHeader& hdr) {
				hdr.seq = nextSeq++;

				// TODO: need msgData.insert
				auto& msg = msgData.insert(hdr.seq);
				msg.data.assign(reinterpret_cast<byte*>(&hdr), reinterpret_cast<byte*>(&hdr) + hdr.size);

				auto* pkt = pktData.find(pktSeq);
				if (!pkt) { pkt = &pktData.insert(pktSeq); }
				pkt->messages.push_back(hdr.seq);

				ENGINE_LOG("RU: ", pktSeq, " ", hdr.seq);
			}

			void recvPacketAck(SeqNum pktSeq) {
				auto* pkt = pktData.find(pktSeq);
				if (!pkt) { return; }

				for (SeqNum s : pkt->messages) {
					auto& m = msgData.get(s);
					if (msgData.find(s)) {
						msgData.remove(s);
						ENGINE_LOG("RU ACK: ", pktSeq, " ", s);
					}
				}

				pktData.remove(pktSeq);
			}

			// TODO: also want some kind of fill-rest-of-packet function
			void writeUnacked(PacketWriter& packetWriter) {
				for (auto s = msgData.min(); seqLess(s, msgData.max()); ++s) {
					auto* msg = msgData.find(s);
					if (msg) {
						ENGINE_LOG("RESEND ");
						packetWriter.write(msg->data.data(), msg->data.size());
					}
				}
			}
	};
}
