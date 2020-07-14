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
						using Index = int32;
						RingBufferImpl* rb;
						SizeType i;

					public:
						using value_type = T;
						using difference_type = Index;
						using reference = T&;
						using pointer = T*;
						using iterator_category = std::random_access_iterator_tag;

					public:
						IteratorBase(RingBufferImpl* rb, SizeType i) : rb{rb}, i{i} {}
						~IteratorBase() = default;

						auto& operator+=(Index n) { i += n; return *this; }
						auto& operator-=(Index n) { return *this += -n; }

						friend auto operator+(IteratorBase it, Index n) { return it += n; }
						friend auto operator-(IteratorBase it, Index n) { return it -= n; }

						friend auto operator+(Index n, IteratorBase it) { return it += n; }
						friend auto operator-(Index n, IteratorBase it) { return it -= n; }

						friend auto operator-(const IteratorBase& a, const IteratorBase& b) { return a.i - b.i; }

						auto& operator++() { return *this += 1; }
						auto& operator--() { return *this -= 1; }

						auto operator++(int) { return *this + 1; }
						auto operator--(int) { return *this - 1; }

						T& operator*() const { return (*rb)[i]; }
						T* operator->() const { return &**this; }
						T& operator[](Index n) const { return *(*this + n); }

						bool operator==(const IteratorBase& other) const { return i == other.i; }
						bool operator!=(const IteratorBase& other) const { return !(*this == other); }
						bool operator<(const IteratorBase& other) const { return i < other.i; }
						bool operator<=(const IteratorBase& other) const { return i <= other.i; }
						bool operator>=(const IteratorBase& other) const { return i >= other.i; }
						bool operator>(const IteratorBase& other) const { return i > other.i; }
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
