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

				template<class T>
				class IteratorBase {
					private:
						RingBufferImpl& rb;
						SizeType i;

					public:
						IteratorBase(RingBufferImpl& rb, SizeType i) : rb{rb}, i{i} {}
						~IteratorBase() = default;

						bool operator==(IteratorBase& other) { return (&rb == &other.rb) && (i == other.i); }
						bool operator!=(IteratorBase& other) { return !(*this == other); }

						IteratorBase& operator+=(int32 n) { i = rb.wrapIndex(i + n); return *this; }
						IteratorBase& operator++() { return *this += 1; }
						IteratorBase operator+(int32 n) { auto other = *this; return other += n; }

						IteratorBase& operator-=(int32 n) { return *this += -n; }
						IteratorBase& operator--() { return *this -= 1; }
						IteratorBase operator-(int32 n) { return *this + -n; }

						T& operator*() { return rb.dataT()[i]; }
						T* operator->() { return &**this; }
				};

				using Iterator = IteratorBase<T>;
				using ConstIterator = IteratorBase<const T>;

			public:
				template<class = std::enable_if_t<IsStatic>>
				RingBufferImpl();

				template<class = std::enable_if_t<!IsStatic>>
				RingBufferImpl(SizeType sz = 16);

				~RingBufferImpl();

				T& operator[](SizeType i);

				const T& operator[](SizeType i) const;

				T& back() noexcept;
				const T& back() const noexcept;

				Iterator begin() noexcept;
				ConstIterator begin() const noexcept;
				ConstIterator cbegin() const noexcept;

				SizeType capacity() const noexcept;

				// TODO: reserve(n)

				void clear();

				template<class... Args>
				void emplace(Args&&... args);

				bool empty() const noexcept;
				
				Iterator end() noexcept;
				ConstIterator end() const noexcept;
				ConstIterator cend() const noexcept;
				
				T& front() noexcept;
				const T& front() const noexcept;

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

				/** The index of the first element */
				SizeType start = 0;

				/** The index of the last element plus one */
				SizeType stop = 0;
				bool isEmpty = true;

				T* dataT() noexcept;

				void elementAdded() noexcept;
				void elementRemoved() noexcept;
				void ensureSpace();
				SizeType wrapIndex(SizeType i);
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
