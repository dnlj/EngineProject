#pragma once

// STD
#include <string>
#include <unordered_map>

// Engine
#include <Engine/Resource.hpp>


namespace Engine {
	// TODO: Split
	// TODO: Doc
	template<class Manager, class T>
	class ResourceManager {
		friend Manager;
		public:
			using ResourceType = Resource<Manager, T>;

			~ResourceManager() {
				for (auto& [path, res] : resources) {
					auto* store = res.storage;
					self().unload(store->data);
					delete store;
				}
			}
			
			ResourceType get(const std::string& path) {
				auto& found = resources[path];
				
				if (found.storage == nullptr) {
					found.storage = new ResourceType::Storage{self().load(path)};
				}
				
				return found;
			};

			void reclaim() {
				for (auto& [path, res] : resources) {
					auto* store = res.storage;

					if (store->clean && store->refCount == 1) {
						self().unload(store->data);
						delete store;
					}

					res.storage = nullptr;
					// TODO: Remove from map?
				}
			}

		private:
			ResourceManager() = default;
			std::unordered_map<std::string, ResourceType> resources;
			Manager& self() { return static_cast<Manager&>(*this); }
	};
}
