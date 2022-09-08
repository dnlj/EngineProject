#pragma once


namespace Engine {
	/**
	 * Pair used for resource reference counting.
	 */
	template<class T>
	class ResourceInfo {
		public:
			int32 refCount = 0;
			T data;

		public:
			template<class... Args>
			ResourceInfo(Args... args) : data(std::forward<Args>(args)...) {
				static_assert(offsetof(ResourceInfo, refCount) == 0,
					"`refCount` is required to be at the same address as `this`. See ResourceRefImpl<T> for details."
				);
				static_assert(offsetof(ResourceInfo, data) >= sizeof(decltype(refCount)),
					"`refCount` is required to be at the same address as `this`. See ResourceRefImpl<T> for details."
				);
			}
	};

	/**
	 * The implementation of ResourceRef.
	 * Split into base class to make extension/specialization easier.
	 * @see ResourceRef
	 */
	template<class T>
	class ResourceRefImpl {
		public:
			using ResourceInfo = ResourceInfo<T>;

		private:
			ResourceInfo* info = nullptr;

			// This is technically undefined behaviour if `T` is not a standard layout type.
			// I guess the "correct" way to do this would be manually malloc our own block
			// of memory where we store a `int32` and an manually aligned `T` separately.
			// Our static_assert in ResourceInfo should catch any issues though.
			void inc() { ++reinterpret_cast<int32&>(*info); }
			void dec() { --reinterpret_cast<int32&>(*info); }

		public:
			/**
			 * Provides access to the underlying ResourceInfo<T>.
			 * Does NOT provide any reference counting.
			 */
			static ResourceInfo* unsafe_getInfo(ResourceRefImpl& ref) noexcept { return ref.info; }
			static const ResourceInfo* unsafe_getInfo(const ResourceRefImpl& ref) noexcept { return ref.info; }

		public:
			ResourceRefImpl() = default;
			ResourceRefImpl(ResourceInfo* info) : info{info} { inc(); };
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

			const T* get() const noexcept { return &info->data; }
			T* get() noexcept { return &info->data; }

			const T* operator->() const noexcept { return get(); }
			T* operator->() noexcept { return get(); }

			const T& operator*() const noexcept { return *get(); }
			T& operator*() noexcept { return *get(); }

			explicit operator bool() const noexcept { return info; }
			const auto count() const noexcept { return info->refCount; }

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
