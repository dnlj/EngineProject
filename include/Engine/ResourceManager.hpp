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

			class Resource {
				private:
					struct ResourceInfo* info = nullptr;
					void inc() { ++info->refCount; }
					void dec() { ++info->refCount; }

				public:
					Resource() = default;
					Resource(ResourceInfo* s) : info{s} {};
					~Resource() { dec(); }
					Resource(const Resource& other) { *this = other; }
					Resource& operator=(const Resource& other) {
						if (this == &other) { return *this; }
						info = other.info;
						inc();
						return *this;
					}
					const auto& get() const { return info->data; }
					const auto& path() const { return info->path; }
					const auto& id() const { return info->id; }
			};

		private:
			struct ResourceInfo {
				const std::string path;
				const ResourceId id;
				int32 refCount = 0;
				T data;
			};

			ResourceManager() = default;
			Engine::FlatHashMap<std::string, ResourceId> resMap;
			std::vector<std::unique_ptr<ResourceInfo>> resInfo;

		public:
			~ResourceManager() {};

			void add(const std::string& path) {
				auto [it, succ] = resMap.try_emplace(path, static_cast<ResourceId>(resInfo.size()));
				if (succ) {
					resInfo.emplace_back();
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

				auto& info = resInfo[found->second];
				if (!info) {
					info.reset(new ResourceInfo{
						.path = path,
						.id = found->second,
						.data = std::move(self().load(path)),
					});
				}

				return info.get();
			}

			Resource get(ResourceId rid) {
				return resInfo[rid].get();
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
			Manager& self() { return static_cast<Manager&>(*this); }
	};
}
