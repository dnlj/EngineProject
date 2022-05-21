#pragma once


namespace Engine {
	using ResourceId2 = int32;

	template<class T>
	class ResourceInfo2 {
		public:
			int32 refCount = 0;
			T data;
	};

	template<class T>
	class Resource2 { // TODO: rename
		private:
			ResourceInfo2<T>* info = nullptr;
			void inc() { ++info->refCount; }
			void dec() { ++info->refCount; }

		public:
			using Id = ResourceId2;
			Resource2() = default;
			Resource2(ResourceInfo2<T>* info) : info{info} { inc(); };
			~Resource2() { dec(); }
			Resource2(const Resource2& other) { *this = other; inc(); }
			Resource2& operator=(const Resource2& other) {
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

	template<class T>
	class ResourceManager2 { // TODO: rename
		public:
			struct CreateResult {
				ResourceId2 id;
				Resource2<T>& obj;
			};

		private:
			std::vector<ResourceId2> reuse;
			std::vector<std::unique_ptr<ResourceInfo2<T>>> infos;

		public:
			template<class... Args>
			CreateResult create(Args&&... args) {
				ResourceId2 id;

				if (!reuse.empty()) {
					id = reuse.back();
					reuse.pop_back();
				} else {
					id = infos.size();
					infos.emplace_back();
				}

				infos[id] = std::make_unique<ResourceInfo2>(std::forward<Args>(args)...);

				return {
					.id = id,
					.obj = Resource2{infos[id].get()},
				};
			}

			void clean() {
				const auto sz = infos.size();
				for (ResourceId2 i = 0; i < sz; ++i) {
					const auto& info = infos[i];
					ENGINE_DEBUG_ASSERT(info->refCount >= 0 || info->refCount > 0xFFFF, "Invalid resource reference count. Something went wrong.");
					if (info->refCount == 0) {
						destroy(i);
					}
				}
			}

		private:
			void destroy(ResourceId2 id) {
				ENGINE_DEBUG_ASSERT(id >= 0 && id < infos.size(), "Attempting to free invalid Resource");
				infos[id] = nullptr;
				reuse.push_back(id);
			}
	};


}
