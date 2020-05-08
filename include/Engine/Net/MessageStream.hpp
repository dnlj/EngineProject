#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/UDPSocket.hpp>
#include <Engine/Net/IPv4Address.hpp>
#include <Engine/Net/Packet.hpp>
#include <Engine/Net/MessageHeader.hpp>


namespace Engine::Net {
	struct Channel {
		enum Type : MessageChannel {
			UNRELIABLE,
			RELIABLE,
			ORDERED,
			_COUNT,
		};
	};

	class MessageStream {
		private:
			UDPSocket& sock;
			IPv4Address addr;
			SequenceNumber nextSeqNum[Channel::_COUNT] = {};

			char* curr;
			char* last;
			Packet packet;

		public:
			static constexpr int32 MAX_MESSAGE_SIZE = sizeof(Packet::data) - sizeof(MessageHeader);

		public:
			MessageStream(UDPSocket& socket);
			MessageStream(const MessageStream&) = delete;
			MessageStream(const MessageStream&&) = delete;

			void next(MessageType type, MessageChannel channel);

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
			void reset(IPv4Address addr, int32 sz = 0);

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
			MessageStream& operator<<(const T& t);

			/**
			 * Reads from the current message.
			 * @see read
			 */
			template<class T>
			MessageStream& operator>>(T& t);

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
	};
}

#include <Engine/Net/MessageStream.ipp>
