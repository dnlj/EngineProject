#pragma once

// Engine
#include <Engine/ResourceMemory.hpp>


namespace Engine {
	/**
	 * Owns memory for a ref counter and object.
	 * @see ResourceManager
	 * @see ResourceRef
	 */
	template<class T>
	class ResourceStorage : public ResourceMemory {
		public:
			template<class... Args>
			static ResourceStorage create(Args&&... args) {
				// Verify all alignment needs
				static_assert(alignof(T) <= alignof(CountType));
				static_assert(alignof(CountType) <= __STDCPP_DEFAULT_NEW_ALIGNMENT__);

				ResourceMemory mem{new byte[sizeof(CountType) + sizeof(T)]};
				mem.getRefCount() = 0;
				new (mem.getObjAddr()) T(std::forward<Args>(args)...);

				return mem;
			}

			ENGINE_INLINE static void destroy(ResourceMemory mem) {
				if (!mem) { return; }
				mem.getObj<T>().~T();
				delete[] mem.memory;
			}

		public:
			ResourceStorage() = delete;
			ResourceStorage(ResourceMemory mem) : ResourceMemory{mem} {}

			ResourceStorage(const ResourceStorage&) = delete;
			ResourceStorage(ResourceStorage&& other) { *this = std::move(other); }

			ResourceStorage& operator=(const ResourceStorage&) = delete;
			ResourceStorage& operator=(ResourceStorage&& other) { swap(*this, other); }

			~ResourceStorage() { destroy(*this); }

			void clear() { *this = ResourceStorage{nullptr}; }

			friend void swap(ResourceStorage& a, ResourceStorage& b) noexcept { std::swap(a.memory, b.memory); }
	};
}
