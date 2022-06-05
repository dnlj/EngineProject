#pragma once


namespace Engine {
	/**
	 * Pair used for resource reference counting.
	 */
	template<class T>
	class ResourceInfo {
		public:
			template<class... Args>
			ResourceInfo(Args... args) : data(std::forward<Args>(args)...) {}

			int32 refCount = 0;
			T data;
	};

	/**
	 * A pointer like type the refers to a resource.
	 */
	template<class T>
	class ResourceRef {
		private:
			ResourceInfo<T>* info = nullptr;
			void inc() { ++info->refCount; }
			void dec() { --info->refCount; }

		public:
			ResourceRef() = default;
			ResourceRef(ResourceInfo<T>* info) : info{info} { inc(); };
			~ResourceRef() { dec(); }

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

			const auto* get() const noexcept { return &info->data; }
			auto* get() noexcept { return &info->data; }

			const auto* operator->() const noexcept { return get(); }
			auto* operator->() noexcept { return get(); }

			const auto& operator*() const noexcept { return *get(); }
			auto& operator*() noexcept { return *get(); }

			operator bool() const noexcept { return info; }

			const auto* _debug() const noexcept { return info; }
	};

	/**
	 * Manages a resource's lifetime.
	 */
	template<class T>
	class ResourceManager { // TODO: rename
		public:
			using ResourceId = uint32;
			using ResourceRef = ResourceRef<T>;
			using ResourceInfo = ResourceInfo<T>;

		private:
			std::vector<ResourceId> reuse;
			std::vector<std::unique_ptr<ResourceInfo>> infos;

		public:
			template<class... Args>
			ResourceRef create(Args&&... args) {
				ResourceId id;

				if (!reuse.empty()) {
					id = reuse.back();
					reuse.pop_back();
				} else {
					id = static_cast<ResourceId>(infos.size());
					infos.emplace_back();
				}
				ENGINE_INFO("[ResourceManager] Create resource ", typeid(T).name(), " ", sizeof...(args));

				infos[id] = std::make_unique<ResourceInfo>(std::forward<Args>(args)...);

				return infos[id].get();
			}

			void clean() {
				const auto sz = infos.size();
				for (ResourceId i = 0; i < sz; ++i) {
					const auto& info = infos[i];
					ENGINE_DEBUG_ASSERT(info->refCount >= 0 || info->refCount > 0xFFFF, "Invalid resource reference count. Something went wrong.");
					if (info->refCount == 0) {
						ENGINE_LOG("Cleaning resource: ", i); // TODO: remove once tested
						destroy(i);
					}
				}
			}

		private:
			void destroy(ResourceId id) {
				ENGINE_DEBUG_ASSERT(id >= 0 && id < infos.size(), "Attempting to free invalid Resource");
				ENGINE_INFO("[ResourceManager] Destroy resource ", typeid(T).name(), " ", id);
				infos[id] = nullptr;
				reuse.push_back(id);
			}
	};

	/**
	 * Loads a resource from a specific key (file path, uri, or similar).
	 */
	template<class Key_, class Resource_>
	class ResourceLoader {
		public:
			using Key = Key_;
			using Resource = Resource_;
			using ResourceRef = ResourceRef<Resource>;
			using Manager = ResourceManager<Resource>;

		private:
			FlatHashMap<Key, ResourceRef> lookup;
			Manager& manager;

		public:
			ResourceLoader(Manager& manager)
				: manager{manager} {
			}

			ResourceRef get(const Key& key) {
				auto found = lookup.find(key);
				if (found == lookup.end()) {
					ENGINE_INFO("[ResourceLoader] Create resource ", typeid(Resource).name());
					const auto& ref = manager.create(load(key));
					found = lookup.try_emplace(key, ref).first;
				}

				ENGINE_INFO("[ResourceLoader] Get resource \"", typeid(Resource).name(), "\" ", found->second._debug()->refCount);
				return found->second;
			}

			// TODO: unload/free/release/etc

		private:
			virtual Resource load(const Key& key) = 0;
	};
}
