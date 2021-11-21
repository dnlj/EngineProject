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

			ENGINE_INLINE ArrayView(T* data, ptrdiff_t size)
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
			
			// TODO: slice

			ENGINE_INLINE bool empty() const noexcept { return dataBegin == dataEnd; }
			ENGINE_INLINE auto size() const noexcept { return dataEnd - dataBegin; }

			ENGINE_INLINE T* data() noexcept { return dataBegin; }
			ENGINE_INLINE const T* data() const noexcept { return dataBegin; }
	};
}
