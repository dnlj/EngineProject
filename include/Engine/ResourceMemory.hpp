#pragma once


namespace Engine {
	/**
	 * Wraps a pointer to a ref counter and object.
	 * @see ResourceRef
	 * @see ResourceStorage
	 */
	class ResourceMemory {
		public:
			using CountType = uint64;
			byte* memory = nullptr;

		public:
			ResourceMemory() = default;
			ResourceMemory(byte* m) : memory{m} {}

			ENGINE_INLINE CountType& getRefCount() noexcept { return *reinterpret_cast<CountType*>(memory); }
			ENGINE_INLINE const CountType& getRefCount() const noexcept { return const_cast<ResourceMemory*>(this)->getRefCount(); }

			ENGINE_INLINE void* getObjAddr() noexcept { return memory + sizeof(CountType); }
			ENGINE_INLINE const void* getObjAddr() const noexcept { return const_cast<ResourceMemory*>(this)->getObjAddr(); }

			template<class T> ENGINE_INLINE T& getObj() noexcept { return *reinterpret_cast<T*>(getObjAddr()); };
			template<class T> ENGINE_INLINE const T& getObj() const noexcept { return const_cast<ResourceMemory*>(this)->getObj<T>(); };

			ENGINE_INLINE operator bool() const noexcept { return memory; }
	};
}
