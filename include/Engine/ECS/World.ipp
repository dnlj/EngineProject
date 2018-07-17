#pragma once

// Engine
#include <Engine/Algorithm/Algorithm.hpp>

namespace Engine::ECS {
	template<class SystemsSet, class ComponentsSet>
	World<SystemsSet, ComponentsSet>::World()
		: SystemManager{*this} {
	}

	template<class SystemsSet, class ComponentsSet>
	Entity World<SystemsSet, ComponentsSet>::createEntity(bool forceNew) {
		const auto ent = EntityManager::createEntity(forceNew);

		if (ent.id >= componentBitsets.size()) {
			componentBitsets.resize(ent.id + 1);
		}

		onEntityCreated(ent);

		return ent;
	}

	template<class SystemsSet, class ComponentsSet>
	void World<SystemsSet, ComponentsSet>::destroyEntity(Entity ent) {
		EntityManager::destroyEntity(ent);
		onEntityDestroyed(ent);
	}

	template<class SystemsSet, class ComponentsSet>
	template<class Component>
	Component& World<SystemsSet, ComponentsSet>::addComponent(Entity ent) {
		auto& container = getComponentContainer<Component>();
		const auto cid = getComponentID<Component>();

		// Ensure the container is of the correct size
		if (ent.id >= container.size()) {
			container.resize(ent.id + 1);
		}

		// Add the component
		container[ent.id] = Component();
		componentBitsets[ent.id][cid] = true;

		// Tell the systems
		onComponentAdded(ent, cid);

		return container[ent.id];
	}

	template<class SystemsSet, class ComponentsSet>
	template<class... Components>
	std::tuple<Components&...> World<SystemsSet, ComponentsSet>::addComponents(Entity ent) {
		return std::forward_as_tuple(addComponent<Components>(ent) ...);
	}

	template<class SystemsSet, class ComponentsSet>
	bool World<SystemsSet, ComponentsSet>::hasComponent(Entity ent, ComponentID cid) {
		return componentBitsets[ent.id][cid];
	}

	template<class SystemsSet, class ComponentsSet>
	template<class Component>
	bool World<SystemsSet, ComponentsSet>::hasComponent(Entity ent) {
		return hasComponent(ent, getComponentID<Component>());
	}

	template<class SystemsSet, class ComponentsSet>
	bool World<SystemsSet, ComponentsSet>::hasComponents(Entity ent, ComponentBitset cbits) {
		return (componentBitsets[ent.id] & cbits) == cbits;
	}

	template<class SystemsSet, class ComponentsSet>
	template<class... Components>
	bool World<SystemsSet, ComponentsSet>::hasComponents(Entity ent) {
		return hasComponents(ent, getBitsetForComponents<Components...>());
	}

	template<class SystemsSet, class ComponentsSet>
	template<class Component>
	void World<SystemsSet, ComponentsSet>::removeComponent(Entity ent) {
		const auto cid = getComponentID<Component>();

		componentBitsets[ent.id][cid] = false;
		onComponentRemoved(ent, cid);
	}

	template<class SystemsSet, class ComponentsSet>
	template<class... Components>
	void World<SystemsSet, ComponentsSet>::removeComponents(Entity ent) {
		(removeComponent<Components>(ent), ...);
	}

	template<class SystemsSet, class ComponentsSet>
	template<class Component>
	Component& World<SystemsSet, ComponentsSet>::getComponent(Entity ent) {
		return getComponentContainer<Component>()[ent.id];
	}

	template<class SystemsSet, class ComponentsSet>
	template<class... Components>
	std::tuple<Components&...> World<SystemsSet, ComponentsSet>::getComponents(Entity ent) {
		return std::forward_as_tuple(getComponent<Components>(ent) ...);
	}

	template<class SystemsSet, class ComponentsSet>
	template<class... Components>
	const EntityFilter& World<SystemsSet, ComponentsSet>::getFilterFor() {
		// TODO: Handle the caase there sizeof...(Components) == 0
		return getFilterFor<Components...>(std::index_sequence_for<Components...>());
	}
	
	template<class SystemsSet, class ComponentsSet>
	template<class... Components, std::size_t... Is>
	const EntityFilter& World<SystemsSet, ComponentsSet>::getFilterFor(std::index_sequence<Is...>) {
		// Sort the argument order so we dont have duplicate filters
		constexpr auto order = Algorithm::sort(
			std::array<ComponentID, sizeof...(Components)>{World::getComponentID<Components>() ...}
		);
	
		return getFilterFor<order[Is]...>();
	}
	
	template<class SystemsSet, class ComponentsSet>
	template<ComponentID C1, ComponentID... Cs>
	const EntityFilter& World<SystemsSet, ComponentsSet>::getFilterFor() {
		auto& con = filters[C1];
	
		ComponentBitset cbits;
		(cbits.set(Cs), ...);
	
		// TODO: sort for faster search?
		auto found = std::find_if(con.cbegin(), con.cend(), [&cbits](const EntityFilter& f) {
			return f.componentBits == cbits;
		});
	
		if (found == con.cend()) {
			con.emplace_back(std::move(cbits));
			auto& back = con.back();

			// Populate the new filter
			for (decltype(Entity::id) i = 0; i < aliveEntities.size(); ++i) {
				const auto gen = aliveEntities[i];

				if (gen > 0) {
					Entity ent{i, gen};

					if (hasComponents(ent, cbits)) {
						back.addEntity(ent);
					}
				}
			}

			return back;
		} else {
			return *found;
		}
	}
}
