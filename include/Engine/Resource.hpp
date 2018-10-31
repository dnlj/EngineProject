#pragma once


namespace Engine {
	// TODO: Split
	// TODO: Doc
	template<class T>
	class Resource {
		template<class, class> friend class ResourceManager;

		public:
			Resource() = default;
			
			~Resource() {
				decrement();
			};

			Resource(const Resource& other) {
				storage = other.storage;
				increment();
			}

			Resource& operator=(const Resource& other) {
				if (this != &other) {
					storage = other.storage;
					increment();
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

			void increment() {
				if (storage != nullptr) {
					++storage->refCount;
				}
			}

			void decrement() {
				if (storage != nullptr) {
					--storage->refCount;
				}
			}

			Storage* storage = nullptr;
	};
}
