#pragma once

// STD
#include <vector>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/UDPSocket.hpp>
#include <Engine/Net/IPv4Address.hpp>
#include <Engine/Net/Packet.hpp>
#include <Engine/Net/MessageHeader.hpp>
#include <Engine/Clock.hpp>
#include <Engine/StaticVector.hpp>


namespace Engine::Net {
	inline constexpr int32 MAX_UNACKED_MESSAGES = 64;
	inline constexpr int32 MAX_MESSAGE_SIZE = sizeof(Packet::data) - sizeof(MessageHeader);
	
	// TODO: name
	// TODO: move
	constexpr SequenceNumber seqToIndex(SequenceNumber seq);
	
	//////////////////////////////////////////////////////////////////////////////////////////////////
	// TODO: move
	struct AckData {
		SequenceNumber nextAck = 0;
		uint64 acks = 0;
		// TODO: some kind of memory pool and views instead? this seems dumb
		std::vector<char> messages[MAX_UNACKED_MESSAGES] = {};
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// TODO: move
	class PacketWriter {
		public: // TODO: not public
			UDPSocket& sock;
			IPv4Address addr;

			char* curr = nullptr;
			char* last = nullptr;
			Packet packet;

			SequenceNumber nextSeq[static_cast<int32>(Channel::_COUNT)] = {};
			AckData channelAckData[2] = {};

		public:
			PacketWriter(UDPSocket& sock, IPv4Address addr);
			PacketWriter(const PacketWriter&) = delete;
			PacketWriter(PacketWriter&&) = delete;

			MessageHeader& header();
			const MessageHeader& header() const;

			// TODO: doc
			void updateSentAcks(Channel ch, SequenceNumber nextAck, uint64 acks);

			/**
			 * Sends this packet to @p addr. Does not modify this packet.
			 * Useful if you wish to send the same packet to multiple addresses.
			 * It is recommended to always #reset or #clear once all sending is done.
			 * 
			 * @returns The number of bytes sent.
			 */
			int32 sendto();
			
			/**
			 * Sends this packet to the address specified the last time #reset was called.
			 * Also resets this packet.
			 * 
			 * @returns The number of bytes sent.
			 */
			int32 send();
			
			/**
			 * Sends any remaining data in this packet.
			 * 
			 * @returns The number of bytes sent.
			 * @see send
			 */
			int32 flush();

			/**
			 * Gets the size of the current message.
			 * @see data
			 */
			int32 size() const;

			/**
			 * Gets the size of the internal data buffer.
			 * @see current
			 */
			static constexpr int32 capacity();

			// TODO: doc
			bool next(MessageType type, Channel channel);

			/**
			 * Writes a specific number of bytes to the current message.
			 */
			void write(const void* t, size_t sz);

			/**
			 * Writes an object to the current message.
			 */
			template<class T>
			void write(const T& t);

			/**
			 * Writes a string to the current message.
			 */
			void write(const std::string& t);

			/**
			 * Writes a string to the current message.
			 */
			void write(const char* t);

			// TODO: doc
			void writeUnacked(Channel ch);

		private:
			void reset(int32 sz = 0);
			void store();
			void endMessage();
			bool canUseChannel(Channel ch) const;
			
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// TODO: move
	class PacketReader {
		public: // TODO: not public
			char* curr = nullptr;
			char* last = nullptr;

			SequenceNumber nextSeq[static_cast<int32>(Channel::_COUNT)] = {}; // TODO: dont think we need this on reader. Only writer
			AckData channelAckData[2] = {};

		public:
			void set(char* curr, char* last);

			// TODO: name
			// TODO: doc
			bool updateRecvAcks(const MessageHeader& hdr);

			/**
			 * Reads a specific number of bytes from the current message.
			 */
			const void* read(size_t sz);
			
			/**
			 * Reads an object from the current message.
			 */
			template<class T>
			decltype(auto) read();

			// TODO: doc
			bool next();

			// TODO: move duplicate reader/writer functions into base class?
			int32 size() const { return static_cast<int32>(last - curr); }
	};

	// TODO: make read functions return ptrs? We need better error handling.
	class Connection {
		public:
			PacketReader reader;
			PacketWriter writer;
			Clock::TimePoint lastMessageTime;

		public:
			Connection(UDPSocket& sock, IPv4Address addr = {}, Clock::TimePoint lastMessageTime = {});
			Connection(const Connection&) = delete;
			Connection(const Connection&&) = delete;

			void writeRecvAcks(Channel ch);

			/**
			 * Gets the most recently associated address.
			 * Set from either #reset or #recv.
			 */
			const IPv4Address& address() const; // TODO: is this used anywhere?
	};
}

#include <Engine/Net/Connection.ipp>
