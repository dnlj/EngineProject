#pragma once

// STD
#include <string>
#include <memory>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/FlatHashMap.hpp>


namespace Engine {
	// TODO: Doc
	template<class Manager, class T>
	class ResourceManager {
		friend Manager;
		public:
			using ResourceId = int32;
			using StorePtr = std::unique_ptr<T>;

		private:
			// TODO: Dont use ptr if T is < some size
			struct ResourceInfo {
				const std::string path;
				const ResourceId id;
				int32 refCount = 0;
				StorePtr data;
			};

			Engine::FlatHashMap<std::string, ResourceId> resMap;
			std::vector<std::unique_ptr<ResourceInfo>> resInfo;

		public:
			class Resource {
				private:
					ResourceInfo* info = nullptr;
					void inc() { ++info->refCount; }
					void dec() { ++info->refCount; }

				public:
					using Id = ResourceId;
					Resource() = default;
					Resource(ResourceInfo& i) : info{&i} { inc(); };
					~Resource() { dec(); }
					Resource(const Resource& other) { *this = other; inc(); }
					Resource& operator=(const Resource& other) {
						if (this == &other) { return *this; }
						info = other.info;
						inc();
						return *this;
					}

					const auto* get() const { return info->data.get(); }
					const auto& path() const { return info->path; }
					const auto& id() const { return info->id; }

					const auto* operator->() const { return get(); }
					const auto& operator*() const { return *get(); }
			};

		public:
			~ResourceManager() {};

			void add(const std::string& path) {
				auto [it, succ] = resMap.try_emplace(path, static_cast<ResourceId>(resInfo.size()));
				if (succ) {
					resInfo.emplace_back(new ResourceInfo{
						.path = path,
						.id = it->second,
					});
				} else {
					ENGINE_WARN("Attempting to add duplicate resource: ", path);
				}
			}

			// TODO: canonicalize + normalize paths to avoid duplicates
			Resource get(const std::string& path) {
				auto found = resMap.find(path);
				if (found == resMap.end()) {
					ENGINE_ERROR("Unable to load invalid resource: ", path);
				}
				return get(found->second);
			}

			Resource get(ResourceId rid) {
				auto& info = resInfo[rid];
				if (!info->data) {
					info->data = std::move(self().load(info->path));
				}

				return *info;
			};

			// TODO: cleanup unreferenced resources
			//void reclaim() {
			//	for (auto& [path, res] : resources) {
			//		auto* store = res.storage;
			//
			//		if (store->clean && store->refCount == 1) {
			//			self().unload(store->data);
			//			delete store;
			//		}
			//
			//		res.storage = nullptr;
			//	}
			//}
		private:
			ResourceManager() = default;
			Manager& self() { return static_cast<Manager&>(*this); }
	};
}
