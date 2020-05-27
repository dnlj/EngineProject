#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine {
	// TODO: largely untested
	// TODO: doc all
	namespace detail {
		template<class T, uint32 Size = 0>
		class RingBufferImpl {
			public:
				constexpr static bool IsStatic = Size != 0;
				using size_type = decltype(Size);
				using SizeType = size_type;

				template<class = std::enable_if_t<IsStatic>>
				RingBufferImpl();

				template<class = std::enable_if_t<!IsStatic>>
				RingBufferImpl(SizeType sz = 16);

				~RingBufferImpl();

				T& back() noexcept;

				const T& back() const noexcept;

				SizeType capacity() const noexcept;

				// TODO: reserve(n)

				void clear();

				template<class... Args>
				void emplace(Args&&... args);

				bool empty() const noexcept;

				bool full() const noexcept;

				void pop();

				void push(const T& obj);
			
				void push(T&& obj);
			
				SizeType size() const noexcept;

				void swap(RingBufferImpl& other);

			private:
				// TODO: we need to store size somewhere
				std::conditional_t<IsStatic,
					char[sizeof(T) * Size],
					std::pair<char*, SizeType> // TODO: unique_ptr
				> data;

				SizeType start = 0;
				SizeType stop = 0;
				bool isEmpty = true;

				T* dataT() noexcept;

				void elementAdded() noexcept;
				void elementRemoved() noexcept;
				void ensureSpace();
		};

		template<class T>
		void swap(RingBufferImpl<T, 0>& first, RingBufferImpl<T, 0>& second) { first.swap(second); };
	}

	template<class T>
	class RingBuffer : public detail::RingBufferImpl<T> {
		using RingBufferImpl::RingBufferImpl;
	};

	template<class T, uint32 Size>
	class StaticRingBuffer : public detail::RingBufferImpl<T, Size> {
		using RingBufferImpl::RingBufferImpl;
	};


}

#include <Engine/StaticRingBuffer.ipp>
