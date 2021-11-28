#pragma once

// Engine
#include <Engine/Engine.hpp>


namespace Engine {
	template<class T>
	class AlignedStorage {
		private:
			alignas(T) byte storage[sizeof(T)];

		public:
			[[nodiscard]] ENGINE_INLINE byte* data() noexcept { return storage; }
			[[nodiscard]] ENGINE_INLINE const byte* data() const noexcept { return storage; }

			[[nodiscard]] ENGINE_INLINE T& as() noexcept { return *reinterpret_cast<T*>(storage); }
			[[nodiscard]] ENGINE_INLINE const T& as() const noexcept { return *reinterpret_cast<T*>(storage); }
	};
}
