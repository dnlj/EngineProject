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
			ResourceInfo(Args... args) : data(std::forward<Args>(args)...) {}
	};

	/**
	 * A pointer like type the refers to a resource.
	 */
	template<class T>
	class ResourceRef {
		public:
			using ResourceInfo = ResourceInfo<T>;

		private:
			ResourceInfo* info = nullptr;
			void inc() { ++info->refCount; }
			void dec() { --info->refCount; }

		public:
			/**
			 * Provides access to the underlying ResourceInfo<T>.
			 * Does NOT provide any reference counting.
			 */
			static ResourceInfo* unsafe_getInfo(ResourceRef& ref) noexcept { return ref.info; }
			static const ResourceInfo* unsafe_getInfo(const ResourceRef& ref) noexcept { return ref.info; }

		public:
			ResourceRef() = default;
			ResourceRef(ResourceInfo* info) : info{info} { inc(); };
			~ResourceRef() { if (info) { dec(); } }

			// Rvalue version doesnt really get us anything because we still need to `dec` our
			// old value and in cases of self assignment we then need to `inc` again. So it would
			// end up looking the same or very similar to the copy version.
			ResourceRef(const ResourceRef& other) { *this = other; }
			ResourceRef& operator=(const ResourceRef& other) {
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

			operator bool() const noexcept { return info; }
			const auto count() const noexcept { return info->refCount; }
	};
}
