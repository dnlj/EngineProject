#pragma once

// STD
#include <array>
#include <vector>
#include <memory>
#include <unordered_map>

// Engine
#include <Engine/ECS/EntityFilter.hpp>
#include <Engine/ECS/Common.hpp>
#include <Engine/ECS/EntityManager.hpp>


// TODO: Test
// TODO: Doc
// TODO: Not happy with this interface.
namespace Engine::ECS {
	class FilterManager {
		public:
			FilterManager(const EntityManager& entityManager);

			template<class World>
			EntityFilter& getFilterFor(const World& world, const ComponentBitset& components);

			void onComponentAdded(Entity ent, ComponentID cid, const ComponentBitset& cbits);
			void onComponentRemoved(Entity ent, ComponentID cid);
			void onEntityDestroyed(Entity ent, const ComponentBitset& cbits);

		private:
			const EntityManager& entityManager;
			std::unordered_map<ComponentBitset, std::unique_ptr<EntityFilter>> filters; // TODO: Use FlatHashMap
			std::vector<std::vector<EntityFilter*>> filtersByComponentID;
	};
}

#include <Engine/ECS/FilterManager.ipp>
