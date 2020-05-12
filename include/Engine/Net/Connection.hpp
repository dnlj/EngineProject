#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/UDPSocket.hpp>
#include <Engine/Net/IPv4Address.hpp>
#include <Engine/Net/Packet.hpp>
#include <Engine/Net/MessageHeader.hpp>
#include <Engine/Clock.hpp>
#include <Engine/StaticVector.hpp>


namespace Engine::Net {
	// TODO: make read functions return ptrs? We need better error handling.
	class Connection {
		private:
			UDPSocket& sock;
			IPv4Address addr;

			char* curr;
			char* last;
			Packet packet;

			static constexpr int32 MAX_UNACKED_MESSAGES = 64;

			struct AckData {
				SequenceNumber nextAck = 0;
				uint64 acks = 0;
				// TODO: some kind of memory pool and views instead? this seems dumb
				std::unique_ptr<char[]> messages[MAX_UNACKED_MESSAGES];
			};

			SequenceNumber nextSeq[static_cast<int32>(Channel::_COUNT)] = {};

			AckData sentAckData[2] = {};
			AckData recvAckData[2] = {}; // TODO: we dont need `messsages[#]` for recv reliable data. Maybe a better way to store datas

		public:
			static constexpr int32 MAX_MESSAGE_SIZE = sizeof(Packet::data) - sizeof(MessageHeader);
			Clock::TimePoint lastMessageTime;

		public:
			Connection(UDPSocket& sock, IPv4Address addr = {}, Clock::TimePoint lastMessageTime = {});
			Connection(const Connection&) = delete;
			Connection(const Connection&&) = delete;

			bool next(MessageType type, Channel channel);

			void updateSentAcks(Channel ch, SequenceNumber nextAck, uint64 acks);

			bool updateRecvAcks(const MessageHeader& hdr);

			void writeRecvAcks(Channel ch);

			// TODO: header field operations
			MessageHeader& header();
			const MessageHeader& header() const;

			/**
			 * Gets the address of the internal data buffer.
			 * Not be confused with the address of the current message.
			 * 
			 * @see #current
			 */
			char* data();

			/** @copydoc data */
			const char* data() const;

			/**
			 * Gets the address of the data buffer for the next message.
			 * Not be confused with the address of the internal data buffer.
			 * 
			 * @see #data
			 */
			char* current();

			/** @copydoc current */
			const char* current() const;

			/**
			 * Gets the most recently associated address.
			 * Set from either #reset or #recv.
			 */
			const IPv4Address& address() const;

			/**
			 * Gets the next packet from the associated UDPSocket.
			 * 
			 * @returns Then number of bytes received.
			 * @see UDPSocket::recv
			 */
			int32 recv();

			/**
			 * Sends this packet to @p addr. Does not modify this packet.
			 * Useful if you wish to send the same packet to multiple addresses.
			 * It is recommended to always #reset or #clear once all sending is done.
			 * 
			 * @returns The number of bytes sent.
			 */
			int32 sendto(const IPv4Address& addr) const;
			
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
			 * Resets this packet's data and 
			 */
			void reset(IPv4Address addr, int32 sz = 0); // TODO: does it make sense for this to be public anymore?

			/**
			 * Clears stream data without sending.
			 * Equivalent to `#reset({0,0,0,0,0})`
			 * 
			 * @warning It is an error to attempt to write more than `sizeof(Packet::data)` before calling #reset
			 * @see reset
			 */
			void clear();

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

			/**
			 * Writes to the current message.
			 * @see write
			 */
			template<class T>
			Connection& operator<<(const T& t);

			/**
			 * Reads from the current message.
			 * @see read
			 */
			template<class T>
			Connection& operator>>(T& t);

			/**
			 * Writes a specific number of bytes to the current message.
			 */
			template<class T>
			void write(const T* t, size_t sz);

			/**
			 * Writes an object to the current message.
			 */
			template<class T>
			void write(const T& t);

			/**
			 * Writes an array to the current message.
			 */
			template<class T, size_t N>
			void write(const T(&t)[N]);

			/**
			 * Writes an array to the current message.
			 */
			template<class T, size_t N>
			void write(const std::array<T, N>& t);

			/**
			 * Writes a string to the current message.
			 */
			void write(const std::string& t);

			/**
			 * Reads a specific number of bytes from the current message.
			 */
			template<class T> 
			void read(T* t, size_t sz);

			/**
			 * Reads an array from the current message.
			 */
			template<class T, size_t N>
			void read(T(&t)[N]);
			
			/**
			 * Reads an array from the current message.
			 */
			template<class T, size_t N>
			void read(std::array<T, N>& t);
			
			/**
			 * Reads a string from the current message.
			 */
			void read(std::string& t);
			
			/**
			 * Reads an object from the current message.
			 */
			template<class T>
			void read(T& t);
			
			/**
			 * Reads an object from the current message.
			 */
			template<class T>
			auto read();

		private:
			void reset(int32 sz = 0);
			bool canUseChannel(Channel ch) const;
			void store();
			constexpr static SequenceNumber seqToIndex(SequenceNumber seq);
	};
}

#include <Engine/Net/Connection.ipp>
