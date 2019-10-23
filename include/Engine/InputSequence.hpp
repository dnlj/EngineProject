#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Input.hpp>
#include <Engine/InputState.hpp>


namespace Engine {
	// TODO: name?
	// TODO: move
	// TODO: split
	// TODO: asserts
	template<class T, uint16_t N>
	class StaticVector {
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

			// TODO: would be nice to have a size constructor
			template<class... Vals>
			StaticVector(Vals&&... vals) : storage{std::forward<Vals>(vals)...}, used{sizeof...(Vals)} {
				static_assert(sizeof...(Vals) <= N, "Too many values given");
			}

			// TODO: overloads with default value
			void resize(size_type count) noexcept {
				//ENGINE_ASSERT(count <= capacity(), "Cannot resize larger than capacity() = " << capacity());
				used = count;
			}

			// TODO: clear

			// TODO: front()

			T& back() noexcept {
				return storage[used - 1];
			}

			const T& back() const noexcept {
				return storage[used - 1];
			}

			// TODO: push_back
			// TODO: pop_back

			// TODO: at

			size_type size() const noexcept {
				return used;
			}

			T* data() noexcept {
				return storage;
			}

			bool empty() const noexcept {
				return used == 0;
			}

			constexpr static size_type capacity() noexcept {
				return N;
			}

			constexpr static size_type max_size() noexcept {
				return capacity();
			}

			T& operator[](const size_type i) noexcept {
				return storage[i];
			}

			const T& operator[](const size_type i) const {
				return storage[i];
			}

			iterator begin() noexcept {
				return storage;
			}

			const_iterator begin() const noexcept {
				return storage;
			}

			const_iterator cbegin() const noexcept {
				return storage;
			}

			iterator end() noexcept {
				return storage + used;
			}

			const_iterator end() const noexcept {
				return storage + used;
			}

			const_iterator cend() const noexcept {
				return storage + used;
			}

			// TODO: (c)rbegin()
			// TODO: (c)rend()
	};

	// TODO: do we want to do this?
	//struct InputSequence : StaticVector<Input, 4> {};
	using InputSequence = StaticVector<Input, 4>;
	using InputStateSequence = StaticVector<InputState, 4>; // TODO: move
}
