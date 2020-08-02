#pragma once

// Engine
#include <Engine/Net/MessageHeader.hpp>
#include <Engine/Net/PacketWriter.hpp>
#include <Engine/Clock.hpp>


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
				while (seqLess(lowest + capacity(), next)) {
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

			ENGINE_INLINE bool contains(S seq) const {
				const auto& e = entries[index(seq)];
				return (e.valid && (e.seq == seq));
			}

			ENGINE_INLINE T* find(S seq) {
				return contains(seq) ? &get(seq) : nullptr;
			}

			ENGINE_INLINE const T* find(S seq) const {
				return const_cast<SeqBuffer*>(this)->find(seq);
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

			constexpr static bool shouldProcess(const MessageHeader& hdr) noexcept {
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
				Engine::Clock::TimePoint lastSendTime;
				std::vector<byte> data;
			};
			SeqBuffer<SeqNum, MsgData, 64> msgData; // TODO: ideal size?

			struct PacketData {
				std::vector<SeqNum> messages;
			};
			SeqBuffer<SeqNum, PacketData, 64> pktData; // TODO: ideal size?

			void addMessageToPacket(SeqNum pktSeq, SeqNum msgSeq) {
				auto* pkt = pktData.find(pktSeq);
				if (!pkt) { pkt = &pktData.insert(pktSeq); }
				pkt->messages.push_back(msgSeq);
			}

		public:
			bool canWriteMessage() const {
				return msgData.spaceFor(nextSeq);
			}

			void msgEnd(SeqNum pktSeq, MessageHeader& hdr) {
				hdr.seq = nextSeq++;

				ENGINE_DEBUG_ASSERT(msgData.spaceFor(hdr.seq));
				auto& msg = msgData.insert(hdr.seq);
				msg.data.assign(reinterpret_cast<byte*>(&hdr), reinterpret_cast<byte*>(&hdr) + sizeof(hdr) + hdr.size);
				msg.lastSendTime = Engine::Clock::now();

				addMessageToPacket(pktSeq, hdr.seq);

				ENGINE_LOG("RU - msgEnd: ", pktSeq, " ", hdr.seq);
			}

			void recvPacketAck(SeqNum pktSeq) {
				auto* pkt = pktData.find(pktSeq);
				if (!pkt) { return; }

				for (SeqNum s : pkt->messages) {
					if (msgData.find(s)) {
						msgData.remove(s);
						ENGINE_LOG("RU - ACK: ", pktSeq, " ", s, " ", msgData.max() - msgData.min());
					}
				}

				pktData.remove(pktSeq);
			}

			
			bool shouldProcess(const MessageHeader& hdr) const {
				return msgData.find(hdr.seq);
			}

			// TODO: also want some kind of fill-rest-of-packet function
			void writeUnacked(PacketWriter& packetWriter) {
				const auto now = Engine::Clock::now();
				// TODO: BUG: at our current call rate this may overwrite packets before we have a chance to ack them because they are overwritten with a new packets info
				for (auto seq = msgData.min(); seqLess(seq, msgData.max() + 1); ++seq) {
					auto* msg = msgData.find(seq);

					// TODO: resend time should be configurable
					if (msg && (now < msg->lastSendTime + std::chrono::milliseconds{50})) {
						//ENGINE_LOG("RU - resend: ", seq);
						packetWriter.ensurePacketAvailable(); // TODO: seems kinda hacky
						packetWriter.write(msg->data.data(), msg->data.size());
						addMessageToPacket(packetWriter.getNextSeq() - 1, seq);
					}
				}
			}
	};
}
