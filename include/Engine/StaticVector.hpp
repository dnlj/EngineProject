#pragma once

// STD
#include <concepts>
#include <type_traits>


namespace Engine {
	// TODO: should probably change this to actually use placement new and ~T() so obj lifetime is managed as expected. (or create another class that works like that)
	template<class T, uint16_t N>
	class StaticVector {
		static_assert(N > 0, "Number of elements must be non-zero");

		private:
			T storage[N];
			uint16_t used = 0;

		public:
			using value_type = T;
			using size_type = decltype(N);
			using difference_type = ptrdiff_t;
			using reference = T&;
			using const_reference = const T&;
			using pointer = T*;
			using const_pointer = const T*;
			using iterator = pointer; // TODO: create iterator type?
			using const_iterator = const_pointer; // TODO: create iterator type?
			// using reverse_iterator
			// using const_reverse_iterator

		public:
			StaticVector() = default;
			StaticVector(StaticVector const&) = default;
			StaticVector(StaticVector&&) = default;

			// TODO: would be nice to have a size constructor
			template<class... Vals, class = std::enable_if_t<(std::is_assignable_v<Vals, value_type> && ...)>>
			StaticVector(Vals&&... vals) : storage{std::forward<Vals>(vals)...}, used{sizeof...(Vals)} {
				static_assert(sizeof...(Vals) <= N, "Too many values given");
			}

			// TODO: overloads with default value
			ENGINE_INLINE void resize(size_type count) noexcept {
				//ENGINE_ASSERT(count <= capacity(), "Cannot resize larger than capacity() = " << capacity());
				used = count;
			}

			ENGINE_INLINE void clear() noexcept {
				resize(0);
			}

			ENGINE_INLINE T& front() noexcept {
				return storage[0];
			}

			ENGINE_INLINE const T& front() const noexcept {
				return storage[0];
			}

			ENGINE_INLINE T& back() noexcept {
				return storage[used - 1];
			}

			ENGINE_INLINE const T& back() const noexcept {
				return storage[used - 1];
			}

			ENGINE_INLINE size_type size() const noexcept {
				return used;
			}

			ENGINE_INLINE T* data() noexcept {
				return storage;
			}

			ENGINE_INLINE const T* data() const noexcept {
				return storage;
			}

			ENGINE_INLINE bool empty() const noexcept {
				return used == 0;
			}

			ENGINE_INLINE constexpr bool full() const noexcept {
				return size() != capacity();
			}

			ENGINE_INLINE constexpr bool space() const noexcept {
				return !full();
			}

			ENGINE_INLINE constexpr static size_type capacity() noexcept {
				return N;
			}

			ENGINE_INLINE constexpr static size_type max_size() noexcept {
				return capacity();
			}

			ENGINE_INLINE T& operator[](const size_type i) noexcept {
				return storage[i];
			}

			ENGINE_INLINE const T& operator[](const size_type i) const {
				return storage[i];
			}

			ENGINE_INLINE iterator begin() noexcept {
				return storage;
			}

			ENGINE_INLINE const_iterator begin() const noexcept {
				return storage;
			}

			ENGINE_INLINE const_iterator cbegin() const noexcept {
				return storage;
			}

			ENGINE_INLINE iterator end() noexcept {
				return storage + used;
			}

			ENGINE_INLINE const_iterator end() const noexcept {
				return storage + used;
			}

			ENGINE_INLINE const_iterator cend() const noexcept {
				return storage + used;
			}

			// TODO: (c)rbegin()
			// TODO: (c)rend()

			ENGINE_INLINE void expand(size_type n = 1) noexcept {
				ENGINE_DEBUG_ASSERT(size() + n <= capacity());
				resize(size() + n);
			}

			ENGINE_INLINE void push_back(T t) {
				ENGINE_DEBUG_ASSERT(size() < capacity());
				expand();
				back() = t;
			}

			ENGINE_INLINE void pop_back() {
				ENGINE_DEBUG_ASSERT(size() > 0);
				resize(size() - 1);
			}

			ENGINE_INLINE void erase(iterator it) {
				std::move(it + 1, end(), it);
				pop_back();
			}

			// TODO: this is wrong. Need more checks around safe bounds for move.
			// TODO: Should change to placement new before messing with this
			//ENGINE_INLINE_REL void insert(iterator it, T&& value) {
			//	if (!space()) { expand(); }
			//
			//	[[maybe_unused]] auto b = begin();
			//	auto e = end();
			//	if (it == e) { return; }
			//
			//	const auto out = it+1;
			//	if (out != e) { std::move(it, e-1, out); }
			//	*it = std::move(value);
			//}

			// TODO: at
	};
}
