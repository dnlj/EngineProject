#pragma once

// STD
#include <array>
#include <type_traits>
#include <cstring>
#include <string>
#include <vector>

// Engine
#include <Engine/Engine.hpp>


namespace Engine::Net {
	class Message {
		public:
			class Header {
				public:
					uint16 protocol;
					uint8 channel;
					uint8 reserved;
					uint32 sequence;
					uint16 type;
			};

			Header header;
			char data[512 - sizeof(Header)];
	};
	static_assert(sizeof(Message) == 512);

	// TODO: bad name
	class MesssageStream {
		private:
			char* curr;
			char* last;
			Message msg;

		public:
			void reset(int32 sz);
			void reset();

			int32 size() const;

			int32 capacity() const;

			char* data();

			const char* data() const;

			template<class T>
			MesssageStream& operator<<(const T& t);

			template<class T>
			MesssageStream& operator>>(T& t);

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
	};
}

#include <Engine/Net/MesssageStream.ipp>
