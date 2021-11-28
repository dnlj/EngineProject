#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine {
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
						using Buff = std::conditional_t<std::is_const_v<T>, const RingBufferImpl, RingBufferImpl>;
						Buff* rb;
						SizeType i;

					public:
						using value_type = T;
						using difference_type = Index;
						using reference = T&;
						using pointer = T*;
						using iterator_category = std::random_access_iterator_tag;

					public:
						IteratorBase(Buff* rb, SizeType i) : rb{rb}, i{i} {}
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

						[[nodiscard]] bool operator==(const IteratorBase& other) const { return i == other.i; }
						[[nodiscard]] bool operator!=(const IteratorBase& other) const { return !(*this == other); }
						[[nodiscard]] bool operator<(const IteratorBase& other) const { return i < other.i; }
						[[nodiscard]] bool operator<=(const IteratorBase& other) const { return i <= other.i; }
						[[nodiscard]] bool operator>=(const IteratorBase& other) const { return i >= other.i; }
						[[nodiscard]] bool operator>(const IteratorBase& other) const { return i > other.i; }
				};

				using Iterator = IteratorBase<T>;
				using ConstIterator = IteratorBase<const T>;

			public:
				// TODO: requires clause?
				template<class = std::enable_if_t<IsStatic>>
				RingBufferImpl();
				
				// TODO: requires clause?
				template<class = std::enable_if_t<!IsStatic>>
				RingBufferImpl(SizeType sz = 16);

				~RingBufferImpl();

				RingBufferImpl(const RingBufferImpl& other);
				RingBufferImpl(RingBufferImpl&& other);
				auto& operator=(const RingBufferImpl& other);
				auto& operator=(RingBufferImpl&& other);

				T& operator[](SizeType i);

				const T& operator[](SizeType i) const;

				[[nodiscard]] Iterator begin() noexcept;
				[[nodiscard]] ConstIterator begin() const noexcept;
				[[nodiscard]] ConstIterator cbegin() const noexcept;
				
				[[nodiscard]] Iterator end() noexcept;
				[[nodiscard]] ConstIterator end() const noexcept;
				[[nodiscard]] ConstIterator cend() const noexcept;

				[[nodiscard]] SizeType capacity() const noexcept;
				
				[[nodiscard]] SizeType size() const noexcept;

				[[nodiscard]] bool empty() const noexcept;

				[[nodiscard]] bool full() const noexcept;

				[[nodiscard]] T& back() noexcept;
				[[nodiscard]] const T& back() const noexcept;

				[[nodiscard]] T& front() noexcept;
				[[nodiscard]] const T& front() const noexcept;

				template<class... Args>
				void emplace(Args&&... args);

				void push(const T& obj);
			
				void push(T&& obj);

				void pop();

				// TODO: reserve(n)

				void clear();

				void swap(RingBufferImpl& other);

			private:
				// TODO: potential alignment issues?
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
		using detail::RingBufferImpl<T>::RingBufferImpl;
	};

	template<class T, uint32 Size>
	class StaticRingBuffer : public detail::RingBufferImpl<T, Size> {
		using detail::RingBufferImpl<T, Size>::RingBufferImpl;
	};


}

#include <Engine/StaticRingBuffer.ipp>
