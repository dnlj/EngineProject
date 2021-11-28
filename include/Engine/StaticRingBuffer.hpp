#pragma once

// STD
#include <memory>

// Engine
#include <Engine/Engine.hpp>


namespace Engine {
	// TODO: doc all
	namespace detail {
		template<class T, uint32 Size = 0>
		class RingBufferImpl {
			public:
				constexpr static bool IsStatic = Size != 0;
				constexpr static bool IsDynamic = !IsStatic;
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

			private:
				// TODO: potential alignment issues?
				using Storage = std::conditional_t<IsStatic,
					char[sizeof(T) * Size],
					std::pair<void*, SizeType> // TODO: unique_ptr
				>;
				Storage data; // TODO: rename data -> storage

				/** The index of the first element */
				SizeType start = 0;

				/** The index of the last element plus one */
				SizeType stop = 0;

				/** Used to determine if we are empty or full (start == stop in both cases) */
				bool isEmpty = true;

			public:
				RingBufferImpl() requires IsStatic = default;
				
				RingBufferImpl(SizeType sz = 16) requires IsDynamic {
					// TODO: replace with reserve(sz);
					data.second = sz;
					data.first = new byte[sizeof(T) * sz];
				}
				
				~RingBufferImpl(){
					clear();
					if constexpr (IsDynamic) { delete[] data.first; }
				}

				RingBufferImpl(const RingBufferImpl& other) { *this = other; }
				RingBufferImpl(RingBufferImpl&& other) requires IsDynamic { swap(*this, other); }
				
				RingBufferImpl& operator=(const RingBufferImpl& other);
				RingBufferImpl& operator=(RingBufferImpl&& other) requires IsDynamic { swap(*this, other); return *this; }
				
				[[nodiscard]] ENGINE_INLINE T& operator[](SizeType i) noexcept {
					return dataT()[wrapIndex(start + i)];
				}

				[[nodiscard]] ENGINE_INLINE const T& operator[](SizeType i) const noexcept {
					return const_cast<RingBufferImpl&>(*this)[i];
				}

				[[nodiscard]] ENGINE_INLINE Iterator begin() noexcept { return {this, 0}; }
				[[nodiscard]] ENGINE_INLINE ConstIterator begin() const noexcept { return cbegin(); }
				[[nodiscard]] ENGINE_INLINE ConstIterator cbegin() const noexcept{ return {this, 0}; }
				
				[[nodiscard]] ENGINE_INLINE Iterator end() noexcept { return {this, size()}; }
				[[nodiscard]] ENGINE_INLINE ConstIterator end() const noexcept { return cend(); }
				[[nodiscard]] ENGINE_INLINE ConstIterator cend() const noexcept { return {this, size()}; }

				[[nodiscard]] ENGINE_INLINE SizeType capacity() const noexcept {
					if constexpr (IsStatic) { return Size; } else { return data.second; }
				}
				
				[[nodiscard]] SizeType size() const noexcept {
					return stop < start ? stop + capacity() - start : stop - start;
				}

				[[nodiscard]] ENGINE_INLINE bool empty() const noexcept { return isEmpty; }

				[[nodiscard]] ENGINE_INLINE bool full() const noexcept { return start == stop && !isEmpty; }

				[[nodiscard]] ENGINE_INLINE T& back() noexcept {
					ENGINE_DEBUG_ASSERT(!empty(), "RingBufferImpl::back called on empty buffer");
					return dataT()[wrapIndex(stop - 1)];
				}

				[[nodiscard]] ENGINE_INLINE const T& back() const noexcept{
					return reinterpret_cast<RingBufferImpl&>(*this).back();
				}

				[[nodiscard]] ENGINE_INLINE T& front() noexcept {
					ENGINE_DEBUG_ASSERT(!empty(), "RingBufferImpl::front called on empty buffer");
					return dataT()[start];
				}

				[[nodiscard]] ENGINE_INLINE const T& front() const noexcept {
					return reinterpret_cast<RingBufferImpl&>(*this).front();
				}

				template<class... Args>
				void emplace(Args&&... args) {
					ensureSpace();
					new (dataT() + stop) T(std::forward<Args>(args)...);
					elementAdded();
				}

				void push(const T& obj) {
					ensureSpace();
					new (dataT() + stop) T(obj);
					elementAdded();
				}
				
				void push(T&& obj) {
					ensureSpace();
					new (dataT() + stop) T(std::move(obj));
					elementAdded();
				}

				void pop() {
					dataT()[start].~T();
					elementRemoved();
				}

				// TODO: std::destroy(begin(), end()) and update start/stop/empty
				ENGINE_INLINE void clear() { while (!empty()) { pop(); } }
				
				ENGINE_INLINE void reserve(SizeType n) requires IsStatic {
					ENGINE_DEBUG_ASSERT(n <= capacity());
				}
				
				void reserve(SizeType n) requires IsDynamic {
					if (capacity() >= n) { return; }

					Storage temp = {};
					temp.first = new byte[sizeof(T) * n]; // TODO: alignment, probably add func to create a Storage of size n
					temp.second = n;

					const auto sz = size();
					std::uninitialized_move(begin(), end(), static_cast<T*>(temp.first));
					clear();
					start = 0;
					stop = sz;
					delete[] data.first;
					data = temp;
					isEmpty = sz == 0;
				}
				
				friend void swap(RingBufferImpl& first, RingBufferImpl& second) requires IsDynamic {
					using std::swap;
					swap(first.data, second.data);
					swap(first.start, second.start);
					swap(first.stop, second.stop);
					swap(first.isEmpty, second.isEmpty);
				}
				
			private:
				ENGINE_INLINE T* dataT() noexcept {
					if constexpr (IsStatic) {
						return reinterpret_cast<T*>(&data);
					} else {
						return reinterpret_cast<T*>(data.first);
					}
				}

				void elementAdded() noexcept {
					ENGINE_DEBUG_ASSERT(!full(), "Element added to full RingBuffer");
					stop = wrapIndex(++stop);
					isEmpty = false;
				}

				void elementRemoved() noexcept {
					ENGINE_DEBUG_ASSERT(!empty(), "Element removed from empty RingBuffer");
					start = wrapIndex(++start);
					isEmpty = start == stop;
				}

				void ensureSpace();

				[[nodiscard]] ENGINE_INLINE SizeType wrapIndex(SizeType i) const noexcept {
					const auto c = capacity();
					return (i + c) % c;
				}
		};
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
