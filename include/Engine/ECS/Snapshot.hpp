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

					ENGINE_INLINE void stepNextValid() {
						const auto end = filter.getCont().end();
						while (it != end && !filter.canUse(it->first)) {
							++it;
						}
					}

					ENGINE_INLINE void stepPrevValid() {
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

					ENGINE_INLINE auto& operator*() {
						ENGINE_DEBUG_ASSERT(filter.canUse(it->first), "Attempt to dereference invalid iterator.");
						return it->first;
					}

					ENGINE_INLINE decltype(auto) operator->() {
						return &**this;
					}

					ENGINE_INLINE bool operator==(const Iter& other) const noexcept { return it == other.it; }
					ENGINE_INLINE bool operator!=(const Iter& other) const noexcept { return !(*this == other); }
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
			template<template<class> class, class...>
			friend class Snapshot;

			template<class,class>
			friend class SingleComponentFilter;

			std::vector<EntityFilter> filters;
			FlatHashMap<ComponentBitset, int32> cbitsToFilter;
			std::array<std::vector<int32>, sizeof...(Cs)> compToFilter;

		public: // TODO: private
			/** Time currently being ticked */
			Clock::TimePoint tickTime = {};

			/** The current tick being run */
			Tick currTick = -1;
				
			/** TODO: doc */
			EntityStates entities;
				
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
			Snapshot() = default;
			Snapshot(const Snapshot& other) = delete;
			
			template<class C, template<class> class ShouldStore2>
			ENGINE_INLINE void copyComponentContainerIf(const Snapshot<ShouldStore2, Cs...>& other) {
				if constexpr (ShouldStore<C>::value && ShouldStore2<C>::value) {
					getComponentContainer<C>() = other.getComponentContainer<C>();
				}
			}

			// TODO: assign is a strange name since we may have less/more mebers than `other`. some comps may persist while others are copied.
			template<template<class> class ShouldStore2>
			void assign(const Snapshot<ShouldStore2, Cs...>& other) {
				filters = other.filters;
				cbitsToFilter = other.cbitsToFilter;
				compToFilter = other.compToFilter;
				tickTime = other.tickTime;
				currTick = other.currTick;
				entities = other.entities;
				deadEntities = other.deadEntities;
				markedForDeath = other.markedForDeath;
				compBitsets = other.compBitsets;
				(copyComponentContainerIf<Cs>(other), ...);
			}
			
			template<class C, class... Comps>
			decltype(auto) getFilter() {
				if constexpr (sizeof...(Comps) == 0) {
					return SingleComponentFilter<C, Snapshot>{*this};
				} else {
					const auto cbits = getBitsetForComponents<C, Comps...>();
					auto found = cbitsToFilter.find(cbits);

					// TODO: maybe having EntityFilter be more of a "view" class would be better to avoid this `.with` stuff and the accidental copy conern.
					if (found != cbitsToFilter.end()) { return filters[found->second].with(entities); }

					const auto idx = static_cast<int32>(filters.size());
					auto& filter = filters.emplace_back(entities, cbits);
					for (const auto& ent : getFilter<C>()) {
						if (!isAlive(ent)) { continue; }
						filter.add(ent, getComponentsBitset(ent));
					}

					cbitsToFilter[cbits] = idx;
					compToFilter[getComponentId<C>()].push_back(idx);
					(compToFilter[getComponentId<Comps>()].push_back(idx), ...);
					return static_cast<const EntityFilter&>(filter);
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

			ENGINE_INLINE void deferedDestroyEntity(Entity ent) {
				setEnabled(ent, false); // TODO: Will we need a component callback for onDisabled to handle things like physics bodies?
				// TODO: would it be better to sort the list afterward (in World::storeSnapshot for example)? instead of while inserting
				markedForDeath.insert(std::lower_bound(markedForDeath.cbegin(), markedForDeath.cend(), ent), ent);
				ENGINE_LOG("deferedDestroyEntity: ", ent);
			}

			void destroyEntities(const decltype(markedForDeath)& ents) {
				if (!ents.size() || !markedForDeath.size()) { return; }
				const auto aEnd = ents.end();
				auto bEnd = markedForDeath.end();
				auto a = ents.begin();
				auto b = markedForDeath.begin();

				for (;b < bEnd; ++b) {
					while (a < aEnd && *a < *b) { ++a; }
					if (a == aEnd) {
						break;
					}
					if (*a != *b || !isAlive(*b)) {
						ENGINE_LOG("destroyEntities - skip: ", *b);
						continue;
					}
					ENGINE_LOG("destroyEntities - dest: ", *b);
					destroyEntity(*b);
					--bEnd;
					std::swap(*b, *bEnd);
				}

				markedForDeath.erase(bEnd, markedForDeath.end());
			}

			// TODO: shouldnt all the isXYZ entity functions also check generation?
			ENGINE_INLINE bool isValid(Entity ent) const noexcept {
				return (ent.id < entities.size())
					&& (ent.gen <= entities[ent.id].ent.gen);
			}

			ENGINE_INLINE bool isAlive(Entity ent) const noexcept {
				const auto& es = entities[ent.id];
				return (es.ent.gen == ent.gen)
					&& (es.state & EntityState::Alive);
			}

			ENGINE_INLINE bool isEnabled(Entity ent) const noexcept {
				const auto& es = entities[ent.id];
				return (es.ent.gen == ent.gen)
					&& (es.state & EntityState::Enabled);
			}

			ENGINE_INLINE void setEnabled(Entity ent, bool enabled) noexcept {
				auto& state = entities[ent.id].state;
				state = (state & ~EntityState::Enabled) | (enabled ? EntityState::Enabled : EntityState::Dead);
			}

			ENGINE_INLINE bool isNetworked(Entity ent) const noexcept {
				const auto& es = entities[ent.id];
				return (es.ent.gen == ent.gen)
					&& (es.state & EntityState::Network);
			}

			ENGINE_INLINE void setNetworked(Entity ent, bool enabled) noexcept {
				auto& state = entities[ent.id].state;
				state = (state & ~EntityState::Network) | (enabled ? EntityState::Network : EntityState::Dead);
			}

			ENGINE_INLINE auto& getEntities() const noexcept {
				return entities;
			}
			
			template<class Component>
			ENGINE_INLINE constexpr static ComponentId getComponentId() noexcept {
				static_assert((std::is_same_v<Cs, Component> || ...),
					"Attempting to get component id of type that is not in the component list. Did you forget to add it?"
				);
				return Meta::IndexOf<Component, Cs...>::value;
			}
			
			/**
			 * Gets the bitset with the bits that correspond to the ids of the given components set.
			 */
			template<class... Components>
			ENGINE_INLINE constexpr static ComponentBitset getBitsetForComponents() noexcept {
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

			ENGINE_INLINE const ComponentBitset& getComponentsBitset(Entity ent) const noexcept {
				return compBitsets[ent.id];
			}

			template<class Component>
			ENGINE_INLINE Component& getComponent(Entity ent) {
				// TODO: why is this not a compile error? this should need `decltype(auto)` return type?
				if constexpr (IsFlagComponent<Component>::value) {
					return compBitsets[ent][getComponentId<Component>()];
				} else {
					ENGINE_DEBUG_ASSERT(hasComponent<Component>(ent), "Attempting to get a component that an entity doesn't have.");
					return getComponentContainer<Component>()[ent];
				}
			}

			template<class Component>
			ENGINE_INLINE const Component& getComponent(Entity ent) const {
				return const_cast<Snapshot*>(this)->getComponent<Component>(ent);
			}

			template<class... Components>
			ENGINE_INLINE std::tuple<Components&...> getComponents(Entity ent) {
				return std::forward_as_tuple(getComponent<Components>(ent) ...);
			}

			ENGINE_INLINE bool hasComponent(Entity ent, ComponentId cid) const {
				//ENGINE_LOG("??? - ", ent, " ", cid);
				return compBitsets[ent.id].test(cid);
			}

			template<class C>
			ENGINE_INLINE bool hasComponent(Entity ent) const {
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

			template<class C>
			ENGINE_INLINE const ComponentContainer<C>& getComponentContainer() const {
				return std::get<ComponentContainer<C>>(compContainers);
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

			
			ENGINE_INLINE ComponentBitset& getComponentsBitset(Entity ent) noexcept {
				return compBitsets[ent.id];
			}

			/**
			 * Destroys and entity, freeing its id to be recycled.
			 */
			void destroyEntity(Entity ent) {
				ENGINE_WARN("destroyEntity: ", ent);
				((hasComponent<Cs>(ent) && (removeComponents<Cs>(ent), 0)), ...);
		
				#if defined(DEBUG)
					if (!isAlive(ent)) {
						ENGINE_ERROR("Attempting to destroy an already dead entity \"", ent, "\"");
					} else if (entities[ent.id].ent.gen != ent.gen) {
						ENGINE_ERROR(
							"Attempting to destroy an old generation entity. Current generation is ",
							entities[ent.id].ent.gen,
							" attempted to delete ",
							ent.gen
						);
					}
				#endif

				deadEntities.push_back(ent);
				entities[ent.id].state = EntityState::Dead;
			}

	};
}
