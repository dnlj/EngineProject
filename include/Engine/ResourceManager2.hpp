#pragma once


namespace Engine {
	using ResourceId2 = int32; // TODO: we really shouldnt need resource ids. atm its only used for networking i think. we should instead just use the key value or similar.

	/**
	 * Pair used for resource reference counting.
	 */
	template<class T>
	class ResourceInfo2 {
		public:
			int32 refCount = 0;
			T data;
	};

	/**
	 * A pointer like type the refers to a resource.
	 */
	template<class T>
	class ResourceRef { // TODO: rename
		private:
			ResourceInfo2<T>* info = nullptr;
			void inc() { ++info->refCount; }
			void dec() { ++info->refCount; }

		public:
			using Id = ResourceId2;
			ResourceRef() = default;
			ResourceRef(ResourceInfo2<T>* info) : info{info} { inc(); };
			~ResourceRef() { dec(); }
			ResourceRef(const ResourceRef& other) { *this = other; inc(); }
			ResourceRef& operator=(const ResourceRef& other) {
				if (this == &other) { return *this; }
				info = other.info;
				inc();
				return *this;
			}

			const auto* get() const { return &info->data; }
			const auto& path() const { return info->path; }
			const auto& id() const { return info->id; }

			const auto* operator->() const { return get(); }
			const auto& operator*() const { return *get(); }
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
				ResourceRef obj;
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

				infos[id] = std::make_unique<ResourceInfo>(0, std::forward<Args>(args)...);

				return {
					.id = id,
					.obj = ResourceRef{infos[id].get()},
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

			ResourceRef get(ResourceId2 id) {
				return infos[id].get();
			}

		private:
			void destroy(ResourceId2 id) {
				ENGINE_DEBUG_ASSERT(id >= 0 && id < infos.size(), "Attempting to free invalid Resource");
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
					const auto& [id, obj] = manager.create(load(key));
					found = lookup.emplace(key, obj).first;
				}
				return found->second;
			}

		private:
			virtual Resource load(const Key& key) = 0;
	};
}
