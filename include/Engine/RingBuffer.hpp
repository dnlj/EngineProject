#pragma once

// STD
#include <memory>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/AlignedStorage.hpp>


namespace Engine {
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
				// TODO (bXIBu9nt): Look into mmap/VirtualAlloc(2). Should increase perfomance and simplify iterators down to simple pointers.
				using Proxy = AlignedStorage<T>;
				using Storage = std::conditional_t<IsStatic,
					Proxy[Size],
					std::pair<Proxy*, SizeType>
				>;
				Storage storage;

				/** The index of the first/oldest element. */
				SizeType tail = 0;

				/** The index of the last/most recent element plus one. */
				SizeType head = 0;

				/** Used to determine if we are empty or full (tail == head in both cases) */
				bool isEmpty = true;

			public:
				RingBufferImpl() requires IsStatic = default;
				
				RingBufferImpl(SizeType sz = 16) requires IsDynamic {
					// TODO (b4HTs9x0): shouldnt alloc here if no size is passed, remove default and make reserve handle empty case.
					sz = nextSize(sz);
					storage.first = new Proxy[sz];
					storage.second = sz;
				}
				
				~RingBufferImpl(){
					clear();
					if constexpr (IsDynamic) { delete[] storage.first; }
				}

				RingBufferImpl(const RingBufferImpl& other) { *this = other; }
				RingBufferImpl(RingBufferImpl&& other) requires IsDynamic { swap(*this, other); }
				
				RingBufferImpl& operator=(const RingBufferImpl& other){
					clear();
					reserve(other.storage.second);
					for (const auto& v : other) { push(v); }
					return *this;
				}

				RingBufferImpl& operator=(RingBufferImpl&& other) requires IsDynamic { swap(*this, other); return *this; }

				[[nodiscard]] ENGINE_INLINE T& operator[](SizeType i) noexcept {
					return dataT()[wrap(tail + i)];
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
					if constexpr (IsStatic) { return Size; } else { return storage.second; }
				}
				
				[[nodiscard]] SizeType size() const noexcept {
					if (empty()) { return 0; }
					return (head - tail) + (head <= tail ? capacity() : 0);
				}

				[[nodiscard]] ENGINE_INLINE bool empty() const noexcept { return isEmpty; }

				[[nodiscard]] ENGINE_INLINE bool full() const noexcept { return tail == head && !isEmpty; }

				[[nodiscard]] ENGINE_INLINE SizeType next() const noexcept { return head; }

				[[nodiscard]] ENGINE_INLINE T& back() noexcept {
					ENGINE_DEBUG_ASSERT(!empty(), "RingBufferImpl::back called on empty buffer");
					return dataT()[wrap(capacity() + head - 1)];
				}

				[[nodiscard]] ENGINE_INLINE const T& back() const noexcept{
					return reinterpret_cast<RingBufferImpl&>(*this).back();
				}

				[[nodiscard]] ENGINE_INLINE T& front() noexcept {
					ENGINE_DEBUG_ASSERT(!empty(), "RingBufferImpl::front called on empty buffer");
					return dataT()[tail];
				}

				[[nodiscard]] ENGINE_INLINE const T& front() const noexcept {
					return reinterpret_cast<RingBufferImpl&>(*this).front();
				}

				template<class... Args>
				T& emplace(Args&&... args) {
					ensureSpace();
					new (dataT() + head) T(std::forward<Args>(args)...);
					elementAdded();
					return back();
				}

				void push(const T& obj) {
					ensureSpace();
					new (dataT() + head) T(obj);
					elementAdded();
				}
				
				void push(T&& obj) {
					ensureSpace();
					new (dataT() + head) T(std::move(obj));
					elementAdded();
				}

				template<class It>
				void push(const It first, const It last) {
					SizeType len1 = last - first;
					SizeType len2 = 0;
					if (len1 == 0) { return; }

					const SizeType req = size() + len1;
					if (req >= capacity()) { reserve(req); }
				
					if (head + len1 >= capacity()) {
						len2 = wrap(head + len1);
						len1 = len1 - len2;
					}

					ENGINE_INLINE_CALLS {
						std::uninitialized_copy_n(first, len1, dataT() + head);
						head = wrap(head + len1);
						
						std::uninitialized_copy_n(first + len1, len2, dataT() + head);
						head = wrap(head + len2);
					}

					isEmpty = false;
				}

				void pop() {
					dataT()[tail].~T();
					elementRemoved();
				}

				ENGINE_INLINE void clear() {
					// TODO: probably better to manually split and use ptrs
					std::destroy(begin(), end());
					tail = 0;
					head = 0;
					isEmpty = true;
				}

				ENGINE_INLINE void reserve(SizeType n) const noexcept requires IsStatic {
					ENGINE_DEBUG_ASSERT(n <= capacity());
				}
				
				void reserve(SizeType n) requires IsDynamic {
					if (capacity() >= n) { return; }
					n = nextSize(n);

					Storage temp = {};
					temp.first = new Proxy[n];
					temp.second = n;

					const auto sz = size();
					std::uninitialized_move(begin(), end(), &temp.first->as());

					clear();
					head = sz;

					delete[] storage.first;
					storage = temp;
				}

				ENGINE_INLINE void resize(SizeType n) requires IsDynamic {
					// TODO: this doesnt account for downsizing?
					reserve(n);
					while (size() < n) { emplace(); }
				}
				
				friend void swap(RingBufferImpl& first, RingBufferImpl& second) requires IsDynamic {
					using std::swap;
					swap(first.data, second.data);
					swap(first.tail, second.tail);
					swap(first.head, second.head);
					swap(first.isEmpty, second.isEmpty);
				}

				/**
				 * Gets a pointer to our internal data buffer.
				 * Keep in mind that only objects in the range [tail, head) are valid.
				 * @see operator[]
				 * @see begin()
				 */
				ENGINE_INLINE const T* unsafe_dataT() const noexcept { return const_cast<RingBufferImpl*>(this)->dataT(); }

			private:
				ENGINE_INLINE T* dataT() noexcept {
					if constexpr (IsStatic) {
						return reinterpret_cast<T*>(&storage);
					} else {
						return reinterpret_cast<T*>(storage.first);
					}
				}

				void elementAdded() noexcept {
					ENGINE_DEBUG_ASSERT(!full(), "Element added to full RingBuffer");
					head = wrap(++head);
					isEmpty = false;
				}

				void elementRemoved() noexcept {
					ENGINE_DEBUG_ASSERT(!empty(), "Element removed from empty RingBuffer");
					tail = wrap(++tail);
					isEmpty = tail == head;
				}

				ENGINE_INLINE void ensureSpace() const noexcept requires IsStatic {
					ENGINE_DEBUG_ASSERT(!full());
				}

				ENGINE_INLINE void ensureSpace() requires IsDynamic {
					if (!full()) { return; }
					// TODO: should we use a next-pow-2 growth factor for more optimized wrap?
					reserve(storage.second + (storage.second / 2)); // 1.5 Growth factor
				}
				
				[[nodiscard]] ENGINE_INLINE SizeType wrap(SizeType i) const noexcept {
					// TODO: rm this check - we used to have a `i+capacity()` here to allow negative indicies. We no longer support that.
					ENGINE_DEBUG_ASSERT(i < std::numeric_limits<SizeType>::max()/2u, "Negative indicies are not supported.");
					if constexpr (IsStatic && nextSize(Size) != Size) {
						return i % capacity();
					} else {
						return i & (capacity() - 1);
					}
				}

				[[nodiscard]] ENGINE_FLATTEN constexpr static SizeType nextSize(SizeType i) noexcept {
					return std::bit_ceil(i); // Next pow 2
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
