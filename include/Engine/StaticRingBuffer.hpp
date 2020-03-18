#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine {
	// TODO: largely untested
	// TODO: doc all
	template<class T, uint32 Size>
	class StaticRingBuffer {
		public:
			using size_type = decltype(Size);
			using SizeType = size_type;

			StaticRingBuffer() = default;
			~StaticRingBuffer();

			SizeType capacity() const noexcept;

			void clear();

			template<class... Args>
			void emplace(Args&&... args);

			bool empty() const noexcept;

			void pop();

			void push(const T& obj);
			
			void push(T&& obj);
			
			SizeType size() const noexcept;

		private:
			char data[sizeof(T) * Size];
			SizeType start = 0;
			SizeType stop = 0;

			T* dataT() noexcept;

			static constexpr void next(SizeType& value) noexcept;
	};
}

#include <Engine/StaticRingBuffer.ipp>
