#pragma once

// STD
#include <array>
#include <type_traits>
#include <cstring>
#include <string>
#include <vector>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Net/UDPSocket.hpp>
#include <Engine/Net/IPv4Address.hpp>

// TODO: split
// TODO: doc
namespace Engine::Net {
	class PacketHeader {
		public:
			uint16 protocol = 0b0110'1001'1001'0110;
	};
	static_assert(sizeof(PacketHeader) == 2);

	class MessageHeader {
		public:
			uint8 type;
			uint8 flags;
			uint16 _filler;
			uint32 sequence;
	};
	static_assert(sizeof(MessageHeader) == 8);

	class Packet {
		public:
			PacketHeader header;
			char data[512 - sizeof(header)];
	};
	static_assert(sizeof(Packet) == 512);

	class MessageStream {
		private:
			UDPSocket& sock;
			IPv4Address addr;

			char* curr;
			char* last;
			Packet packet;

		public:
			static constexpr int32 MAX_MESSAGE_SIZE = sizeof(Packet::data) - sizeof(MessageHeader);

		public:
			MessageStream(UDPSocket& socket);
			MessageStream(const MessageStream&) = delete;
			MessageStream(const MessageStream&&) = delete;

			void next(MessageHeader head);

			// TODO: header field operations
			MessageHeader& header();
			const MessageHeader& header() const;

			char* data();
			const char* data() const;

			char* current();
			const char* current() const;

			const IPv4Address& address() const;

			int32 recv();

			/**
			 * Sends this packet to @p addr. Does not modify this packet.
			 * Useful if you wish to send the same packet to multiple addresses.
			 * It is recommended to always reset() or clear() once all sending is done.
			 * 
			 * @returns The number of bytes sent.
			 */
			int32 sendto(const IPv4Address& addr) const;
			
			/**
			 * Sends this packet to the address specified the last time reset() was called.
			 * Also resets this packet.
			 * 
			 * @returns The number of bytes sent.
			 */
			int32 send();
			
			/**
			 * Sends any remaining data in this packet.
			 * 
			 * @returns The number of bytes sent.
			 * @see send()
			 */
			int32 flush();

			/**
			 * Resets this packet's data and 
			 */
			void reset(IPv4Address addr, int32 sz = 0);

			/**
			 * Clears stream data without sending.
			 * Equivalent to `reset({0,0,0,0,0})`
			 * 
			 * @warning It is an error to attempt to write more than `sizeof(Packet::data)` before calling reset()
			 * @see reset()
			 */
			void clear();

			int32 size() const;

			static constexpr int32 capacity();

			template<class T>
			MessageStream& operator<<(const T& t);

			template<class T>
			MessageStream& operator>>(T& t);

			template<class T>
			void write(const T* t, size_t sz);

			template<class T>
			void write(const T& t);

			template<class T, size_t N>
			void write(const T(&t)[N]);

			template<class T, size_t N>
			void write(const std::array<T, N>& t);

			void write(const std::string& t);

			template<class T> 
			void read(T* t, size_t sz);

			template<class T, size_t N>
			void read(T(&t)[N]);

			template<class T, size_t N>
			void read(std::array<T, N>& t);

			void read(std::string& t);

			template<class T>
			void read(T& t);

			template<class T>
			auto read();

		private:
			void reset(int32 sz = 0);
	};
}

#include <Engine/Net/MessageStream.ipp>
