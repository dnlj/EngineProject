#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/AckData.hpp>
#include <Engine/Net/MessageHeader.hpp>
#include <Engine/Net/Packet.hpp>
#include <Engine/Net/UDPSocket.hpp>
#include <Engine/Net/MessageHeader.hpp>


namespace Engine::Net {
	inline constexpr int32 MAX_MESSAGE_SIZE = sizeof(Packet::data) - sizeof(MessageHeader);

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
}

#include <Engine/Net/PacketWriter.ipp>
