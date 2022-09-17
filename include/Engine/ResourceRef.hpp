#pragma once

// Engine
#include <Engine/ResourceMemory.hpp>


namespace Engine {
	/**
	 * The implementation of ResourceRef.
	 * Split into base class to make extension/specialization easier.
	 * @see ResourceRef
	 */
	template<class T>
	class ResourceRefImpl {
		private:
			ResourceMemory info{nullptr};

			void inc() { ++info.getRefCount(); }
			void dec() { --info.getRefCount(); }

		public:
			/**
			 * Provides access to the underlying ResourceMemory.
			 * Does NOT provide any reference counting.
			 */
			ENGINE_INLINE static ResourceMemory unsafe_getInfo(ResourceRefImpl& ref) noexcept { return ref.info; }
			ENGINE_INLINE static const ResourceMemory unsafe_getInfo(const ResourceRefImpl& ref) noexcept { return ref.info; }

		public:
			ResourceRefImpl() = default;
			ResourceRefImpl(ResourceMemory info) : info{info} { inc(); };
			~ResourceRefImpl() { if (info) { dec(); } }

			// Rvalue version doesnt really get us anything because we still need to `dec` our
			// old value and in cases of self assignment we then need to `inc` again. So it would
			// end up looking the same or very similar to the copy version.
			ResourceRefImpl(const ResourceRefImpl& other) { *this = other; }
			ResourceRefImpl& operator=(const ResourceRefImpl& other) {
				if (info) { dec(); }
				info = other.info;
				if (info) { inc(); }
				return *this;
			}

			ENGINE_INLINE const T* get() const noexcept { return &info.getObj<T>(); }
			ENGINE_INLINE T* get() noexcept { return &info.getObj<T>(); }

			ENGINE_INLINE const T* operator->() const noexcept { return get(); }
			ENGINE_INLINE T* operator->() noexcept { return get(); }

			ENGINE_INLINE const T& operator*() const noexcept { return *get(); }
			ENGINE_INLINE T& operator*() noexcept { return *get(); }

			ENGINE_INLINE explicit operator bool() const noexcept { return info; }
			ENGINE_INLINE const auto count() const noexcept { return info.getRefCount(); }

			ENGINE_INLINE friend bool operator==(const ResourceRefImpl& lhs, const ResourceRefImpl& rhs) noexcept {
				return lhs.info == rhs.info;
			}

	};
	
	/**
	 * A pointer like type the refers to a resource.
	 * @see ResourceRefImpl
	 */
	template<class T>
	class ResourceRef : public ResourceRefImpl<T> {
		using ResourceRefImpl<T>::ResourceRefImpl;
	};

	template<class T>
	struct Hash<ResourceRef<T>> {
		[[nodiscard]] size_t operator()(const ResourceRef<T>& val) const {
			return reinterpret_cast<uintptr_t>(ResourceRef<T>::unsafe_getInfo(val).memory);
		}
	};
}
