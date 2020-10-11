#pragma once

// Engine
#include <Engine/FlatHashMap.hpp>
#include <Engine/Clock.hpp>
#include <Engine/ECS/Common.hpp>
#include <Engine/ECS/EntityState.hpp>
#include <Engine/ECS/EntityFilter.hpp>


namespace Engine::ECS {
	// TODO: move
	// TODO: make copy constructor on this and EntityFilter private
	template<class C, class Snap>
	class SingleComponentFilter {
		private:
			using Cont = ComponentContainer<C>;
			using It = decltype(std::declval<Cont>().begin());
			using CIt = decltype(std::declval<Cont>().cbegin());
			Snap& snap;
			ENGINE_INLINE auto& getCont() { return snap.getComponentContainer<C>(); }
			// TODO: do we also want to check if enabled? maybe filters need some way to specify what flags an entity should/shouoldnt have
			ENGINE_INLINE bool canUse(Entity ent) const { return snap.isAlive(ent); }

			class Iter {
				private:
					friend class SingleComponentFilter;
					using I = int32;
					It it;
					SingleComponentFilter& filter;

					void stepNextValid() {
						const auto end = filter.getCont().end();
						while (it != end && !filter.canUse(it->first)) {
							++it;
						}
					}

					void stepPrevValid() {
						const auto begin = filter.getCont().begin();
						while (it != begin && !filter.canUse(it->first)) {
							--it;
						}
					}

				public:
					Iter(It it, SingleComponentFilter& filter) : it{it}, filter{filter} {}

					// TODO: need to check that ent is enabled
					auto& operator++() {
						const auto end = filter.getCont().end();
						if (it != end) { ++it; }
						stepNextValid();
						return *this;
					}

					auto& operator--() {
						const auto begin = filter.getCont().begin();
						if (it != begin) { --it; }
						stepPrevValid();
						return *this;
					}

					auto& operator*() {
						ENGINE_DEBUG_ASSERT(filter.canUse(it->first), "Attempt to dereference invalid iterator.");
						return it->first;
					}

					decltype(auto) operator->() {
						return &**this;
					}

					bool operator==(const Iter& other) const noexcept { return it == other.it; }
					bool operator!=(const Iter& other) const noexcept { return !(*this == other); }
			};
		public:
			SingleComponentFilter(Snap& snap) : snap{snap} {}

			Iter begin() {
				Iter it{getCont().begin(), *this};
				it.stepNextValid();
				return it;
			}

			Iter end() {
				return {getCont().end(), *this};
			}

			auto size() {
				int32 i = 0;
				for (auto it = begin(); it != end(); ++it, ++i) {}
				return i;
			}

			bool empty() { return begin() == end(); }
	};

	// TODO: move
	template<class...>
	struct EntityFilterList {};

	template<class T>
	struct IsEntityFilterList : std::false_type {};

	template<class... Cs>
	struct IsEntityFilterList<EntityFilterList<Cs...>> : std::true_type {};

	// TODO: move docs from World. Add @copydoc to World defs
	template<template<class> class ShouldStore, class... Cs>
	class Snapshot {
		private:
			template<class,class> friend class SingleComponentFilter;

			std::vector<EntityFilter<Snapshot>> filters;
			FlatHashMap<ComponentBitset, int32> cbitsToFilter;
			std::vector<int32> compToFilter[sizeof...(Cs)];

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
			
			template<class C, class... Comps>
			decltype(auto) getFilter() {
				if constexpr (sizeof...(Comps) == 0) {
					return SingleComponentFilter<C, Snapshot>{*this};
				} else {
					const auto cbits = getBitsetForComponents<C, Comps...>();
					auto found = cbitsToFilter.find(cbits);
					if (found != cbitsToFilter.end()) { return filters[found->second]; }
					ENGINE_INFO("Creating filter for ", cbits); // TODO: rm

					const auto idx = static_cast<int32>(filters.size());
					auto& filter = filters.emplace_back(*this, cbits);
					for (const auto& ent : getFilter<C>()) {
						if (!isAlive(ent)) { continue; }
						filter.add(ent, getComponentsBitset(ent));
					}

					cbitsToFilter[cbits] = idx;
					compToFilter[getComponentId<C>()].push_back(idx);
					(compToFilter[getComponentId<Comps>()].push_back(idx), ...);
					return filter;
				}
			}

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

			// TODO: add destroyuEntity for world to call. Update filters.

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
				auto& cbits = getComponentsBitset(ent);
				cbits.set(cid);

				auto& container = getComponentContainer<C>();
				container.add(ent, std::forward<Args>(args)...);

				// Update filters
				for (const auto i : compToFilter[cid]) {
					auto& filter = filters[i];
					filter.add(ent, cbits);

					// TODO: rm
					ENGINE_INFO("Adding ", ent, " to filter ", i, " ( C = ", getComponentId<C>(), ")");
				}

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

			template<class... Comps>
			void removeComponents(Entity ent) {
				compBitsets[ent.id] &= ~getBitsetForComponents<Comps...>();
				(getComponentContainer<Comps>().remove(ent), ...);

				// Update filters
				const auto& rm = [&](const auto& is){
					for (auto i : is) {
						filters[i].remove(ent);
						ENGINE_INFO("Removing ", ent, " from filter ", i); // TODO: rm
					}
				};
				(rm(compToFilter[getComponentId<Comps>()]), ...);
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

			
			ComponentBitset& getComponentsBitset(Entity ent) noexcept {
				return compBitsets[ent.id];
			}

	};
}
