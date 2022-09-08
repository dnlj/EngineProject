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
			 * Provides access to the underlying ResourceInfo<T>.
			 * Does NOT provide any reference counting.
			 */
			static ResourceMemory unsafe_getInfo(ResourceRefImpl& ref) noexcept { return ref.info; }
			static const ResourceMemory unsafe_getInfo(const ResourceRefImpl& ref) noexcept { return ref.info; }

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

			const T* get() const noexcept { return &info.getObj<T>(); }
			T* get() noexcept { return &info.getObj<T>(); }

			const T* operator->() const noexcept { return get(); }
			T* operator->() noexcept { return get(); }

			const T& operator*() const noexcept { return *get(); }
			T& operator*() noexcept { return *get(); }

			explicit operator bool() const noexcept { return info; }
			const auto count() const noexcept { return info.getRefCount(); }

	};
	
	/**
	 * A pointer like type the refers to a resource.
	 * @see ResourceRefImpl
	 */
	template<class T>
	class ResourceRef : public ResourceRefImpl<T> {
		using ResourceRefImpl<T>::ResourceRefImpl;
	};
}
