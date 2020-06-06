#pragma once

// STD
#include <array>
#include <vector>
#include <memory>

// Engine
#include <Engine/ECS/EntityFilter.hpp>
#include <Engine/ECS/Common.hpp>
#include <Engine/FlatHashMap.hpp>


// TODO: Test
// TODO: Doc
// TODO: Not happy with this interface.
namespace Engine::ECS {
	template<class World>
	class FilterManager {
		public:
			using Filter = EntityFilter<World>;
			FilterManager();

			Filter& getFilterFor(const World& world, const ComponentBitset& components);

			void onComponentAdded(Entity ent, ComponentId cid, const ComponentBitset& cbits);
			void onComponentRemoved(Entity ent, ComponentId cid);
			void onEntityDestroyed(Entity ent, const ComponentBitset& cbits);

		private:
			FlatHashMap<ComponentBitset, std::unique_ptr<Filter>> filters;
			std::vector<std::vector<Filter*>> filtersByComponentId;
	};
}

#include <Engine/ECS/FilterManager.ipp>
