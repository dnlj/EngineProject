#pragma once


namespace Engine {
	using ResourceId2 = int32; // TODO: we really shouldnt need resource ids. atm its only used for networking i think. we should instead just use the key value or similar.

	/**
	 * Pair used for resource reference counting.
	 */
	template<class T>
	class ResourceInfo2 {
		public:
			template<class... Args>
			ResourceInfo2(Args... args) : data(std::forward<Args>(args)...) {}

			int32 refCount = 0;
			T data;
	};

	/**
	 * A pointer like type the refers to a resource.
	 */
	template<class T>
	class ResourceRef {
		private:
			ResourceInfo2<T>* info = nullptr;
			void inc() { ++info->refCount; }
			void dec() { --info->refCount; }

		public:
			using Id = ResourceId2;
			ResourceRef() = default;
			ResourceRef(ResourceInfo2<T>* info) : info{info} { inc(); };
			ResourceRef(const ResourceRef& other) { *this = other; }
			~ResourceRef() { dec(); }
			ResourceRef& operator=(const ResourceRef& other) {
				if (info) { dec(); }
				info = other.info;
				if (info) { inc(); }
				return *this;
			}

			const auto* get() const { return &info->data; }
			auto* get() { return &info->data; }

			const auto* operator->() const { return get(); }
			auto* operator->() { return get(); }

			const auto& operator*() const { return *get(); }
			auto& operator*() { return *get(); }

			const auto* _debug() const { return info; }
	};

	/**
	 * Manages a resource's lifetime.
	 */
	template<class T>
	class ResourceManager2 { // TODO: rename
		public:
			using ResourceRef = ::Engine::ResourceRef<T>;
			using ResourceInfo = ResourceInfo2<T>;
			struct CreateResult {
				ResourceId2 id;
				ResourceRef ref;
			};

		private:
			std::vector<ResourceId2> reuse;
			std::vector<std::unique_ptr<ResourceInfo>> infos;

		public:
			template<class... Args>
			CreateResult create(Args&&... args) {
				ResourceId2 id;

				if (!reuse.empty()) {
					id = reuse.back();
					reuse.pop_back();
				} else {
					id = static_cast<ResourceId2>(infos.size());
					infos.emplace_back();
				}
				ENGINE_INFO("[ResourceManager2] Create resource ", typeid(T).name(), " ", sizeof...(args));

				infos[id] = std::make_unique<ResourceInfo>(std::forward<Args>(args)...);

				return {
					.id = id,
					.ref = ResourceRef{infos[id].get()},
				};
			}

			void clean() {
				const auto sz = infos.size();
				for (ResourceId2 i = 0; i < sz; ++i) {
					const auto& info = infos[i];
					ENGINE_DEBUG_ASSERT(info->refCount >= 0 || info->refCount > 0xFFFF, "Invalid resource reference count. Something went wrong.");
					if (info->refCount == 0) {
						ENGINE_LOG("Cleaning resource: ", i); // TODO: remove once tested
						destroy(i);
					}
				}
			}

			//ResourceRef get(ResourceId2 id) {
			//	return infos[id].get();
			//}

		private:
			void destroy(ResourceId2 id) {
				ENGINE_DEBUG_ASSERT(id >= 0 && id < infos.size(), "Attempting to free invalid Resource");
				ENGINE_INFO("[ResourceManager2] Destroy resource ", typeid(T).name(), " ", id);
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
			using Resource = Resource_; // TODO: T a better name
			using ResourceRef = ResourceRef<Resource>;
			using Manager = ResourceManager2<Resource>;

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
					const auto& [id, obj] = manager.create(load(key));
					found = lookup.emplace(key, obj).first;
				}

				ENGINE_INFO("[ResourceLoader] Get resource \"", typeid(Resource).name(), "\" ", found->second._debug()->refCount);
				return found->second;
			}

			// TODO: unload/free/release/etc

		private:
			virtual Resource load(const Key& key) = 0;
	};
}
