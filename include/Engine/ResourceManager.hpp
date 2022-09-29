#pragma once

// Engine
#include <Engine/ResourceRef.hpp>
#include <Engine/ResourceStorage.hpp>

namespace Engine {
	/**
	 * Manages a resource's lifetime.
	 */
	template<class T>
	class ResourceManager {
		public:
			using ResourceId = uint32;
			using ResourceRef = ResourceRef<T>;

		private:
			std::vector<ResourceId> reuse;
			std::vector<ResourceStorage<T>> infos;

		public:
			template<class... Args>
			ResourceRef create(Args&&... args) {
				ResourceId id;

				if (!reuse.empty()) {
					id = reuse.back();
					reuse.pop_back();
				} else {
					id = static_cast<ResourceId>(infos.size());
					infos.emplace_back(nullptr);
				}

				infos[id] = ResourceStorage<T>::create(std::forward<Args>(args)...);
				return infos[id];
			}

			void clean() {
				const auto sz = infos.size();
				for (ResourceId i = 0; i < sz; ++i) {
					const auto& info = infos[i];
					ENGINE_DEBUG_ASSERT(info->refCount >= 0 || info->refCount > 0xFFFF, "Invalid resource reference count. Something went wrong.");
					if (info->refCount == 0) {
						ENGINE_WARN("Cleaning resource: ", i); // TODO: remove once tested
						destroy(i);
					}
				}
			}

		private:
			void destroy(ResourceId id) {
				ENGINE_DEBUG_ASSERT(id >= 0 && id < infos.size(), "Attempting to free invalid Resource");
				infos[id].clear();
				reuse.push_back(id);
			}
	};

	/**
	 * Loads a resource from a specific key (file path, uri, or similar).
	 */
	template<class Key_, class Resource_>
	class ResourceLoader : public ResourceManager<Resource_> {
		public:
			using Key = Key_;
			using Resource = Resource_;
			using ResourceRef = ResourceRef<Resource>;
			using Manager = ResourceManager<Resource>;

		private:
			FlatHashMap<Key, ResourceRef> lookup;

		public:
			ResourceRef get(const Key& key) {
				auto found = lookup.find(key);
				if (found == lookup.end()) {
					const auto& ref = this->create(load(key));
					found = lookup.try_emplace(key, ref).first;
				}

				return found->second;
			}

			// TODO: unload/free/release/etc

			void clean() {
				// TODO: impl
				ResourceManager::clean();
			}

			const auto begin() const { return lookup.cbegin(); }
			const auto end() const { return lookup.cend(); }

		private:
			virtual Resource load(const Key& key) = 0;
	};
}
