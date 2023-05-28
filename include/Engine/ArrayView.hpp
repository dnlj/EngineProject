#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine {
	template<class T>
	class ArrayView {
		private:
			T* dataBegin = nullptr;
			T* dataEnd = nullptr;

		public:
			ArrayView() = default;

			ENGINE_INLINE ArrayView(T* begin, T* end)
				: dataBegin{begin}
				, dataEnd{end} {
				ENGINE_DEBUG_ASSERT(dataBegin <= dataEnd);
			};

			ENGINE_INLINE ArrayView(T* data, int64 size)
				: ArrayView(data, data + size) {
			};

			ENGINE_INLINE ArrayView(auto& cont)
				: ArrayView(adl_data(cont), adl_size(cont)) {
			}

			ENGINE_INLINE ArrayView(std::initializer_list<T> list)
				: ArrayView(const_cast<T*>(list.begin()), const_cast<T*>(list.end())) {
				static_assert(std::is_const_v<T>, "std::initializer_list can only be converted to an ArrayView over constant elements: ArrayView<const T>");
			}

			ENGINE_INLINE T* begin() noexcept { return dataBegin; }
			ENGINE_INLINE const T* begin() const noexcept { return dataBegin; }
			ENGINE_INLINE const T* cbegin() const noexcept { return dataBegin; }

			ENGINE_INLINE T* end() noexcept { return dataEnd; }
			ENGINE_INLINE const T* end() const noexcept { return dataEnd; }
			ENGINE_INLINE const T* cend() const noexcept { return dataEnd; }

			// TODO: rbegin
			// TODO: crbegin
			// TODO: rend
			// TODO: crend

			ENGINE_INLINE bool empty() const noexcept { return dataBegin == dataEnd; }
			ENGINE_INLINE auto size() const noexcept { return dataEnd - dataBegin; }

			ENGINE_INLINE T* data() noexcept { return dataBegin; }
			ENGINE_INLINE const T* data() const noexcept { return dataBegin; }

			ENGINE_INLINE T& operator[](int64 i) noexcept { return dataBegin[i]; }
			ENGINE_INLINE const T& operator[](int64 i) const noexcept { return dataBegin[i]; }

			ENGINE_INLINE T& front() noexcept { return *dataBegin; }
			ENGINE_INLINE const T& front() const noexcept { return *dataBegin; }

			ENGINE_INLINE T& back() noexcept { return dataEnd[-1]; }
			ENGINE_INLINE const T& back() const noexcept { return dataEnd[-1]; }

			/**
			 * Creates a new view containing the elements [begin, end).
			 */
			ENGINE_INLINE ArrayView slice(int64 begin, int64 end) const noexcept { return {dataBegin + begin, dataEnd + end}; }
			ENGINE_INLINE ArrayView slice(int64 begin) const noexcept { return {dataBegin + begin, dataEnd}; }

			/**
			 * Creates a new view containing this views first @p n elements.
			 */
			ENGINE_INLINE ArrayView first(int64 n) const noexcept { return {dataBegin, dataBegin + n}; }
			
			/**
			 * Creates a new view containing this views last @p n elements.
			 */
			ENGINE_INLINE ArrayView last(int64 n) const noexcept { return {dataEnd - n, dataEnd}; }
	};

	template<class T>
	ArrayView(std::initializer_list<T>&&) -> ArrayView<const T>;
}
