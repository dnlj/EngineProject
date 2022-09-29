#pragma once

// Engine
#include <Engine/ResourceManager.hpp>
#include <Engine/Gfx/NumberType.hpp>
#include <Engine/Gfx/VertexAttributeLayout.hpp>


namespace Engine::Gfx {
	class VertexLayoutManager : public ResourceManager<VertexAttributeLayout> {
		using ResourceManager::ResourceManager;
	};

	class VertexLayoutCache final {
		private:
			FlatHashMap<uint32, VertexAttributeLayoutRef> lookup;

		public:
			VertexAttributeLayoutRef set(uint32 id, VertexAttributeLayoutRef ref) {
				ENGINE_DEBUG_ASSERT(!lookup.contains(id), "Attempting to map duplicate vertex layout id");
				return lookup[id] = ref;
			}

			VertexAttributeLayoutRef get(uint32 id) {
				auto found = lookup.find(id);
				if (found != lookup.end()) {
					return found->second;
				}
				return {};
			}
	};
}
