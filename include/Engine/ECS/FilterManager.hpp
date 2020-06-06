#pragma once

// STD
#include <array>
#include <vector>
#include <memory>

// Engine
#include <Engine/ECS/EntityFilter.hpp>
#include <Engine/ECS/Common.hpp>
#include <Engine/ECS/EntityManager.hpp>
#include <Engine/FlatHashMap.hpp>


// TODO: Test
// TODO: Doc
// TODO: Not happy with this interface.
namespace Engine::ECS {
	class FilterManager {
		public:
			FilterManager(const EntityManager& entityManager);

			template<class World>
			EntityFilter& getFilterFor(const World& world, const ComponentBitset& components);

			void onComponentAdded(Entity ent, ComponentId cid, const ComponentBitset& cbits);
			void onComponentRemoved(Entity ent, ComponentId cid);
			void onEntityDestroyed(Entity ent, const ComponentBitset& cbits);

		private:
			const EntityManager& entityManager;
			FlatHashMap<ComponentBitset, std::unique_ptr<EntityFilter>> filters;
			std::vector<std::vector<EntityFilter*>> filtersByComponentId;
	};
}

#include <Engine/ECS/FilterManager.ipp>
