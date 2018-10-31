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
		public:
			Resource() {
			};

			~Resource() {
			};

			const T& get() const {
				return storage->data;
			}

		private:
			template<class, class> friend class ResourceManager;
			struct Storage {
				public:
					Storage(T&& other) : data{other} {}
					T data;
					short refCount = 0;
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
			Resource<T> get(const std::string& path) {
				auto& found = resources[path];
				
				if (found.storage == nullptr) {
					found.storage = new Resource<T>::Storage{self().load(path)};
				}
				
				return found;
			};

		private:
			ResourceManager() = default;
			std::unordered_map<std::string, Resource<T>> resources;
			Manager& self() { return static_cast<Manager&>(*this); }
	};
}
