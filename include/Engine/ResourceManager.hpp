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
					struct Storage* storage = nullptr;
					void inc() { ++storage->refCount; }
					void dec() { ++storage->refCount; }

				public:
					Resource() = default;
					Resource(Storage* s) : storage{s} {};
					~Resource() { dec(); }
					Resource(const Resource& other) { *this = other; }
					Resource& operator=(const Resource& other) {
						if (this == &other) { return *this; }
						storage = other.storage;
						inc();
						return *this;
					}
					const T& get() const { return storage->data; }
			};

		private:
			struct Storage {
				T data;
				int32 refCount = 0;
			};

			struct ResourceInfo {
				std::string path;
				Storage* storage;
			};

			ResourceManager() = default;
			Engine::FlatHashMap<std::string, ResourceId> resMap;
			std::vector<ResourceInfo> resInfo;

		public:
			~ResourceManager() {
				for (const auto& info : resInfo) {
					delete info.storage;
				}
			};

			ResourceId add(const std::string& path) {
				auto [it, succ] = resMap.try_emplace(path, static_cast<ResourceId>(resInfo.size()));
				if (succ) {
					resInfo.push_back({path});
				} else {
					ENGINE_WARN("Attempting to add duplicate resource: ", path);
				}
				return it->second;
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

				if (info.storage == nullptr) {
					info.storage = new Storage{self().load(info.path)};
				}
				
				return info.storage;
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
