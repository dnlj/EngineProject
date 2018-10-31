#pragma once

// STD
#include <string>
#include <unordered_map>


namespace Engine {
	// TODO: Move
	// TODO: Split
	// TODO: Doc
	template<class T>
	class Resource {
		template<class, class> friend class ResourceManager;

		public:
			Resource() = default;
			
			~Resource() {
				if (storage != nullptr) {
					--storage->refCount;
				}
			};

			Resource(const Resource& other) {
				storage = other.storage;
				++storage->refCount;
			}

			Resource& operator=(const Resource& other) {
				if (this != &other) {
					storage = other.storage;
					++storage->refCount;
				}

				return *this;
			}

			const T& get() const {
				return storage->data;
			}

		private:
			struct Storage {
				public:
					Storage(T&& other) : data{other} {}
					T data;
					short refCount = 1;
					bool clean = true;
			};

			Storage* storage = nullptr;
	};

	// TODO: Split
	// TODO: Doc
	template<class Manager, class T>
	class ResourceManager {
		friend Manager;
		public:
			// TODO: cleanup all resources in destructor
			
			Resource<T> get(const std::string& path) {
				auto& found = resources[path];
				
				if (found.storage == nullptr) {
					found.storage = new Resource<T>::Storage{self().load(path)};
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
			std::unordered_map<std::string, Resource<T>> resources;
			Manager& self() { return static_cast<Manager&>(*this); }
	};
}
