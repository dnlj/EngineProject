#pragma once

// STD
#include <array>
#include <vector>
#include <memory>
#include <unordered_map>

// Engine
#include <Engine/ECS/EntityFilter.hpp>
#include <Engine/ECS/Common.hpp>


// TODO: Test
// TODO: Doc
// TODO: Not happy with this interface.
namespace Engine::ECS {
	class FilterManager {
		public:
			FilterManager();

			template<class World>
			EntityFilter& getFilterFor(const World& world, const ComponentBitset& components);

		protected:
			void onComponentAdded(Entity ent, ComponentID cid, const ComponentBitset& cbits);
			void onComponentRemoved(Entity ent, ComponentID cid);
			void onEntityDestroyed(Entity ent, const ComponentBitset& cbits);

		private:
			std::unordered_map<ComponentBitset, std::unique_ptr<EntityFilter>> filters;
			std::vector<std::vector<EntityFilter*>> filtersByComponentID;
	};
}

#include <Engine/ECS/FilterManager.ipp>
