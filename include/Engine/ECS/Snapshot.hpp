#pragma once

// Engine
#include <Engine/Clock.hpp>
#include <Engine/ECS/Common.hpp>
#include <Engine/ECS/EntityState.hpp>


namespace Engine::ECS {
	// TODO: move docs from World. Add @copydoc to World defs
	template<template<class> class ShouldStore, class... Cs>
	class Snapshot {
		public: // TODO: private
			/** Time currently being ticked */
			Clock::TimePoint tickTime = {};

			/** The current tick being run */
			Tick currTick = 0;
				
			/** TODO: doc */
			std::vector<EntityState> entities;
				
			/** TODO: doc */
			std::vector<Entity> deadEntities;

			/** TODO: doc */
			std::vector<Entity> markedForDeath;

			/** The bitsets for storing what components entities have. */
			std::vector<ComponentBitset> compBitsets;
			
			/** The containers for storing components. */
			// TODO: dont store non rollback relevant comps at all
			std::tuple<
				std::conditional_t<ShouldStore<Cs>::value, ComponentContainer<Cs>, bool>...
			> compContainers;

		public:
			//template<template<class> class ShouldStore2>
			//Snapshot(const Snapshot<ShouldStore2, Cs...>& other) {
			//	// TODO: untested
			//	((ShouldStore<Cs>::value && ShouldStore2<Cs>::value ? getComponentContainer<Cs> = other.getComponentContainer<Cs>() : void), ...);
			//}

			Entity createEntity(bool forceNew) {
				EntityState* es;

				if (!forceNew && !deadEntities.empty()) {
					const auto i = deadEntities.back().id;
					deadEntities.pop_back();
					es = &entities[i];
				} else {
					es = &entities.emplace_back(Entity{static_cast<decltype(Entity::id)>(entities.size()), 0}, EntityState::Dead);
				}

				++es->ent.gen;
				es->state = EntityState::Alive | EntityState::Enabled;

				if (es->ent.id >= compBitsets.size()) {
					compBitsets.resize(es->ent.id + 1); // TODO: Is one really the best increment size? Doesnt seem right.
				} else {
					compBitsets[es->ent.id].reset();
				}

				return es->ent;
			}

			void deferedDestroyEntity(Entity ent) {
				setEnabled(ent, false); // TODO: Will we need a component callback for onDisabled to handle things like physics bodies?
				markedForDeath.push_back(ent);
			}

			ENGINE_INLINE bool isAlive(Entity ent) const noexcept {
				return entities[ent.id].state & EntityState::Alive;
			}

			ENGINE_INLINE bool isEnabled(Entity ent) const noexcept {
				return entities[ent.id].state & EntityState::Enabled;
			}

			ENGINE_INLINE void setEnabled(Entity ent, bool enabled) noexcept {
				auto& state = entities[ent.id].state;
				state = (state & ~EntityState::Enabled) | (enabled ? EntityState::Enabled : EntityState::Dead);
			}

			ENGINE_INLINE bool isNetworked(Entity ent) const noexcept {
				return entities[ent.id].state & EntityState::Network;
			}

			ENGINE_INLINE void setNetworked(Entity ent, bool enabled) noexcept {
				auto& state = entities[ent.id].state;
				state = (state & ~EntityState::Network) | (enabled ? EntityState::Network : EntityState::Dead);
			}

			ENGINE_INLINE auto& getEntities() const noexcept {
				return entities;
			}
			
			template<class Component>
			constexpr static ComponentId getComponentId() noexcept {
				if constexpr ((std::is_same_v<Cs, Component> || ...)) {
					return Meta::IndexOf<Component, Cs...>::value;
				} else {
					return sizeof...(Cs) + Meta::IndexOf<Component, Fs...>::value;
				}
			}
			
			/**
			 * Gets the bitset with the bits that correspond to the ids of the given components set.
			 */
			template<class... Components>
			constexpr static ComponentBitset getBitsetForComponents() noexcept {
				ComponentBitset value;
				(value.set(getComponentId<Components>()), ...);
				return value;
			}

			template<class C, class... Args>
			decltype(auto) addComponent(Entity ent, Args&&... args) {
				constexpr auto cid = getComponentId<C>();
				ENGINE_DEBUG_ASSERT(!hasComponent<C>(ent), "Attempting to add duplicate component (", cid ,") to ", ent);
				compBitsets[ent.id].set(cid);

				auto& container = getComponentContainer<C>();
				container.add(ent, std::forward<Args>(args)...);
				return container[ent];
			}

			const ComponentBitset& getComponentsBitset(Entity ent) const noexcept {
				return compBitsets[ent.id];
			}

			template<class Component>
			Component& getComponent(Entity ent) {
				// TODO: why is this not a compile error? this should need `decltype(auto)` return type?
				if constexpr (IsFlagComponent<Component>::value) {
					return compBitsets[ent][getComponentId<Component>()];
				} else {
					ENGINE_DEBUG_ASSERT(hasComponent<Component>(ent), "Attempting to get a component that an entity doesn't have.");
					return getComponentContainer<Component>()[ent];
				}
			}

			template<class... Components>
			std::tuple<Components&...> getComponents(Entity ent) {
				return std::forward_as_tuple(getComponent<Components>(ent) ...);
			}

			bool hasComponent(Entity ent, ComponentId cid) {
				return compBitsets[ent.id].test(cid);
			}

			template<class C>
			bool hasComponent(Entity ent) {
				return hasComponent(ent, getComponentId<C>());
			}

			template<class... Components>
			void removeComponents(Entity ent) {
				compBitsets[ent.id] &= ~getBitsetForComponents<Components...>();
				(getComponentContainer<Components>().remove(ent), ...);
			}

		private:
			/**
			 * Get the container for components of type @p Component.
			 * @tparam C The type of the component.
			 * @return A reference to the container associated with @p Component.
			 */
			template<class C>
			ENGINE_INLINE ComponentContainer<C>& getComponentContainer() {
				return std::get<ComponentContainer<C>>(compContainers);
			}

	};
}
