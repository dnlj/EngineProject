#pragma once

// STD
#include <tuple>
#include <type_traits>

// Meta
#include <Meta/IndexOf.hpp>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Clock.hpp>
#include <Engine/ECS/Common.hpp>
#include <Engine/SequenceBuffer.hpp>
#include <Engine/ECS/EntityFilter.hpp>
#include <Engine/FlatHashMap.hpp>


namespace Engine::ECS {
	// TODO: move
	// TODO: make copy constructor on this and EntityFilter private
	template<class C, class World>
	class SingleComponentFilter {
		private:
			using Cont = ComponentContainer<C>;
			using It = decltype(std::declval<Cont>().begin());
			using CIt = decltype(std::declval<Cont>().cbegin());
			World& world;
			ENGINE_INLINE auto& getCont() const { return world.template getComponentContainer<C>(); }

			// TODO: do we also want to check if enabled? maybe filters need some way to specify what flags an entity should/shouoldnt have
			ENGINE_INLINE bool canUse(Entity ent) const {
				return world.isAlive(ent) && world.isEnabled(ent);
			}

			class Iter {
				private:
					friend class SingleComponentFilter;
					using I = int32;
					It it;
					const SingleComponentFilter& filter;

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
					Iter(It it, const SingleComponentFilter& filter) : it{it}, filter{filter} {}

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
			SingleComponentFilter(World& world) : world{world} {}

			Iter begin() const {
				Iter it{getCont().begin(), *this};
				it.stepNextValid();
				return it;
			}

			Iter end() const {
				return {getCont().end(), *this};
			}

			auto size() const {
				int32 i = 0;
				for (auto it = begin(); it != end(); ++it, ++i) {}
				return i;
			}

			bool empty() const { return begin() == end(); }
	};

	// TODO: move
	template<class...>
	struct EntityFilterList {};

	template<class T>
	struct IsEntityFilterList : std::false_type {};

	template<class... Cs>
	struct IsEntityFilterList<EntityFilterList<Cs...>> : std::true_type {};
	
	// TODO: move
	template<class T, class = void>
	struct IsSnapshotRelevant : std::false_type {};

	template<class T>
	struct IsSnapshotRelevant<T, std::void_t<typename T::SnapshotData>> : std::true_type {};
}

namespace Engine::ECS {
	/**
	 * @tparam Derived CRTP dervied class. Needed for EntityFilter<Derived>. // TODO: this may no longer be needed? dont think its used anymore since filter rework.
	 * @tparam TickRate The tick rate of the world.
	 * @tparam SystemsSet The systems for this world to have.
	 * @tparam ComponentsSet The components for entities in this world to have.
	 */
	template<class Derived, int64 TickRate, class SystemsSet, class ComponentsSet>
	class World;

	#define WORLD_TPARAMS template<\
		class Derived,\
		int64 TickRate,\
		class... Ss,\
		template<class...> class SystemsSet,\
		class... Cs,\
		template<class...> class ComponentsSet\
	>

	#define WORLD_CLASS World<Derived, TickRate, SystemsSet<Ss...>, ComponentsSet<Cs...>>
	
	WORLD_TPARAMS
	class WORLD_CLASS {
		static_assert(sizeof...(Cs) <= MAX_COMPONENTS);
		public:
			using Filter = EntityFilter; // TODO: now unneeded. use EntityFilter direct

		private:
			/** TODO: doc */
			bool performingRollback = false;

			/** Beginning of last run. */
			Clock::TimePoint beginTime;

			// TODO: shouldnt this be part of snapshot? that would fix our issue with correct tickTime i think
			// TODO: not public
			/** TODO: doc */
			public: float32 tickScale = 1.0f; private:

			/** How long between each tick. */
			constexpr static Clock::Duration tickInterval{Clock::Period::den / TickRate};

			/** The tick interval in floating point seconds. */
			constexpr static float32 tickDeltaTime = Clock::Seconds{tickInterval}.count();
			
			/** Time it took to process the last run. */
			float32 deltaTime = 0.0f;
			
			/** Delta time in nanoseconds. @see deltaTime */
			Clock::Duration deltaTimeNS{0};

			/** All the systems in this world. */
			std::tuple<Ss...> systems;

		////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////
		private:
			template<class,class>
			friend class SingleComponentFilter;

			std::vector<EntityFilter> filters;
			FlatHashMap<ComponentBitset, int32> cbitsToFilter;
			std::array<std::vector<int32>, sizeof...(Cs)> compToFilter;

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
			std::tuple<ComponentContainer<Cs>...> compContainers;

			struct {
				Engine::ECS::Tick tick = -1;
				Clock::TimePoint time = {};
			} rollbackData;

			struct Snapshot {
				Clock::TimePoint tickTime = {};

				// TODO: how should we handle flag components?
				template<class T, class = void>
				struct CompCont { using Type = bool; };

				template<class T>
				struct CompCont<T, std::enable_if_t<IsSnapshotRelevant<T>::value>> { using Type = SparseSet<Entity, typename T::SnapshotData>; };

				std::tuple<typename CompCont<Cs>::Type ...> compConts;
				template<class C>
				ENGINE_INLINE auto& getComponentContainer() {
					static_assert(IsSnapshotRelevant<C>::value,
						"Attempting to get component container for non-snapshot relevant component."
					);

					return std::get<getComponentId<C>()>(compConts);
				}

				template<class C>
				ENGINE_INLINE const auto& getComponentContainer() const { return const_cast<Snapshot*>(this)->getComponentContainer<C>(); }

			};

			SequenceBuffer<Tick, Snapshot, TickRate> history;
			
		public:
			// TODO: doc
			template<class Arg>
			World(Arg&& arg);

			World(const World&) = delete;

			/**
			 * Advances simulation time and calls the `tick` and `run` members of systems.
			 */
			void run();

			ENGINE_INLINE bool isPerformingRollback() const noexcept { return performingRollback; }
			ENGINE_INLINE void scheduleRollback(Tick t) { rollbackData.tick = t; }
			ENGINE_INLINE bool hasHistory(Tick tick) const { return history.contains(tick); }
			
			////////////////////////////////////////////////////////////////////////////////
			// Entity Functions
			////////////////////////////////////////////////////////////////////////////////
			// TODO: Doc. valid vs alive vs enabled
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

			/**
			 * Creates an entity.
			 * @param forceNew Disables recycling entity ids.
			 */
			Entity createEntity(bool forceNew = false) { // TODO: split
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

			/**
			 * Marks an Entity to be destroyed once it is out of rollback scope.
			 * Until the Enttiy is destroyed it is disabled.
			 */
			ENGINE_INLINE void deferedDestroyEntity(Entity ent) {
				setEnabled(ent, false); // TODO: Will we need a component callback for onDisabled to handle things like physics bodies?
				// TODO: would it be better to sort the list afterward (in World::storeSnapshot for example)? instead of while inserting
				markedForDeath.insert(std::lower_bound(markedForDeath.cbegin(), markedForDeath.cend(), ent), ent);
				ENGINE_LOG("deferedDestroyEntity: ", ent);
			}
			
			/**
			 * Gets all entities.
			 */
			ENGINE_INLINE auto& getEntities() const noexcept { return entities; };

			////////////////////////////////////////////////////////////////////////////////
			// System Functions
			////////////////////////////////////////////////////////////////////////////////
			/**
			 * Gets the id of a system.
			 */
			template<class System>
			constexpr static SystemId getSystemId() noexcept;

			/**
			 * Gets the instance of the system for this world.
			 */
			template<class System>
			System& getSystem();

			/**
			 * Gets a bitset with the bits for the given systems set.
			 */
			template<class... SystemN>
			SystemBitset getBitsetForSystems() const; // TODO: constexpr, noexcept

			////////////////////////////////////////////////////////////////////////////////
			// Component Functions
			////////////////////////////////////////////////////////////////////////////////
			/**
			 * Get the ComponentId associated with a component.
			 * @tparam Component The component.
			 * @return The id of @p C.
			 */
			template<class Component>
			ENGINE_INLINE constexpr static ComponentId getComponentId() noexcept {
				static_assert((std::is_same_v<Cs, Component> || ...),
					"Attempting to get component id of type that is not in the component list. Did you forget to add it?"
				);
				return ::Meta::IndexOf<Component, Cs ...>::value;
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

			/**
			 * Adds a component to an entity.
			 * @param ent The entity.
			 * @param args The arguments to pass to the constructor of the component.
			 * @tparam C The component.
			 * @return A reference to the added component.
			 */
			template<class C, class... Args>
			decltype(auto) addComponent(Entity ent, Args&&... args) { // TODO: split
				constexpr auto cid = getComponentId<C>();
				ENGINE_DEBUG_ASSERT(!hasComponent<C>(ent), "Attempting to add duplicate component (", cid ,") to ", ent);
				auto& cbits = compBitsets[ent.id];
				cbits.set(cid);

				auto& container = getComponentContainer<C>();
				container.add(ent, std::forward<Args>(args)...);

				// Update filters
				for (const auto i : compToFilter[cid]) {
					auto& filter = filters[i];
					filter.add(ent, cbits);
					// ENGINE_INFO("Adding ", ent, " to filter ", i, " ( C = ", getComponentId<C>(), ")");
				}

				return container[ent];
			}

			/**
			 * Adds components to an entity.
			 * @param[in] ent The entity.
			 * @tparam Components The components.
			 * @return A tuple of references to the added components.
			 */
			template<class... Components>
			ENGINE_INLINE decltype(auto) addComponents(Entity ent) { return std::forward_as_tuple(addComponent<Components>(ent) ...); };

			/**
			 * Checks if an entity has a component.
			 * @param[in] ent The entity.
			 * @param[in] cid The id of the component.
			 * @return True if the entity has the component; otherwise false.
			 */
			// TODO: use getComponentsBitset
			ENGINE_INLINE bool hasComponent(Entity ent, ComponentId cid) const { return compBitsets[ent.id].test(cid); }

			/**
			 * Checks if an entity has a component.
			 * @param[in] ent The entity.
			 * @tparam Component The component type.
			 * @return True if the entity has the component; otherwise false.
			 */
			template<class C>
			ENGINE_INLINE bool hasComponent(Entity ent) const { return hasComponent(ent, getComponentId<C>()); }
			
			/**
			 * Removes a component from an entity.
			 * @param[in] ent The entity.
			 * @tparam C The component.
			 */
			template<class C>
			ENGINE_INLINE void removeComponent(Entity ent) { removeComponents<C>(ent); };

			/**
			 * Removes components from an entity.
			 * @param[in] ent The entity.
			 * @tparam Components The components.
			 */
			template<class... Comps>
			void removeComponents(Entity ent) {
				compBitsets[ent.id] &= ~getBitsetForComponents<Comps...>();
				(getComponentContainer<Comps>().remove(ent), ...);

				// Update filters
				const auto& rm = [&](const auto& is){
					for (auto i : is) {
						filters[i].remove(ent);
						// ENGINE_INFO("Removing ", ent, " from filter ", i);
					}
				};
				(rm(compToFilter[getComponentId<Comps>()]), ...);
			}

			/**
			 * Removes all components from an entity.
			 */
			ENGINE_INLINE void removeAllComponents(Entity ent) { ((hasComponent<Cs>(ent) && (removeComponent<Cs>(ent), 1)), ...); };

			/**
			 * Gets a reference to the component instance associated with an entity.
			 */
			template<class Component>
			ENGINE_INLINE Component& getComponent(Entity ent) {
				// TODO: why is this not a compile error? this should need `decltype(auto)` return type?
				static_assert(!IsFlagComponent<Component>::value,
					"Calling World::getComponent on a flag component is not allowed. Use World::hasComponent instead."
				);

				ENGINE_DEBUG_ASSERT(hasComponent<Component>(ent), "Attempting to get a component that an entity doesn't have.");
				return getComponentContainer<Component>()[ent];
			}

			template<class Component>
			ENGINE_INLINE const Component& getComponent(Entity ent) const {
				return const_cast<World*>(this)->getComponent<Component>(ent);
			}

			template<class C>
			ENGINE_INLINE bool hadComponent(Entity ent, Tick tick) const {
				return history.get(tick).getComponentContainer<C>().has(ent);
			}

			// TODO: Doc
			/**
			 * 
			 */
			template<class C>
			ENGINE_INLINE auto& getComponentState(Entity ent, Tick tick) {
				static_assert(IsSnapshotRelevant<C>::value,
					"Attempting to get component from snapshot for non-snapshot relevant component."
				);

				auto& snap = history.get(tick);
				auto& cont = snap.getComponentContainer<C>();
				if (!cont.has(ent)) {
					ENGINE_LOG("Add historic component ", getComponentId<C>(), " to ", ent, " on tick ", tick);
					cont.add(ent);
				}
				return cont[ent];
			}

			template<class Component>
			ENGINE_INLINE const auto& getComponentState(Entity ent, Tick tick) const {
				return const_cast<World*>(this)->getComponentState<Component>(ent, tick);
			}

			/**
			 * Gets a reference the components associated with an entity.
			 * @param[in] ent The entity.
			 * @tparam Components The components.
			 * @return A tuple of references to the components.
			 */
			template<class... Components>
			ENGINE_INLINE std::tuple<Components&...> getComponents(Entity ent) {
				return std::forward_as_tuple(getComponent<Components>(ent) ...);
			}

			template<class... Components>
			ENGINE_INLINE std::tuple<const Components&...> getComponents(Entity ent) const {
				return std::forward_as_tuple(getComponent<Components>(ent) ...);
			}

			/**
			 * Gets the components bitset for an entity.
			 * @param[in] ent The entity.
			 * @return The components bitset for the entity
			 */
			ENGINE_INLINE decltype(auto) getComponentsBitset(Entity ent) const noexcept { return compBitsets[ent.id]; }

			/**
			 * Gets the components bitset for all entities. Sorted by entity id. 
			 */
			ENGINE_INLINE const auto& getAllComponentBitsets() const { return compBitsets; };

			// TODO: doc
			template<class C, class... Comps>
			decltype(auto) getFilter() {
				if constexpr (IsEntityFilterList<C>::value) {
					return [&]<class... Ds>(EntityFilterList<Ds...>) -> decltype(auto) {
						return getFilter<Ds...>();
					}(C{});
				} else if constexpr (sizeof...(Comps) == 0) {
					return SingleComponentFilter<C, World>{*this};
				} else {
					const auto cbits = getBitsetForComponents<C, Comps...>();
					auto found = cbitsToFilter.find(cbits);

					// TODO: maybe having EntityFilter be more of a "view" class would be better to avoid this `.with` stuff and the accidental copy conern.
					if (found != cbitsToFilter.end()) { return filters[found->second].with(entities); }

					const auto idx = static_cast<int32>(filters.size());
					auto& filter = filters.emplace_back(entities, cbits);
					for (const auto& pair : getComponentContainer<C>()) {
						const auto& ent = pair.first;
						if (!isAlive(ent)) { continue; }
						filter.add(ent, getComponentsBitset(ent));
					}

					cbitsToFilter[cbits] = idx;
					compToFilter[getComponentId<C>()].push_back(idx);
					(compToFilter[getComponentId<Comps>()].push_back(idx), ...);
					return static_cast<const EntityFilter&>(filter);
				}
			}
			
			/**
			 * Gets the current tick.
			 */
			ENGINE_INLINE auto getTick() const { return currTick; }

			// TODO: also need to clear rollback history
			// TODO: should this be setNextTick? might help avoid bugs if we wait till all systems are done before we adjust.
			// TODO: doc
			void setNextTick(Tick tick);

			/**
			 * Gets the tick interval.
			 * @see tickInterval
			 */
			ENGINE_INLINE constexpr static auto getTickInterval() { return tickInterval; };

			/**
			 * Gets the tick delta.
			 */
			ENGINE_INLINE constexpr static auto getTickDelta() { return tickDeltaTime; }

			/**
			 * Current time being ticked.
			 */
			ENGINE_INLINE Clock::TimePoint getTickTime() const noexcept { return tickTime; };
			ENGINE_INLINE Clock::TimePoint getTickTime(Tick tick) const noexcept { return history.get(tick).tickTime; };
			
			/**
			 * Gets the time (in seconds) last update took to run.
			 */
			float32 getDeltaTime() const;

			/**
			 * Gets the time (in nanoseconds) last update took to run.
			 */
			auto getDeltaTimeNS() const;

			/**
			 * Checks if SystemA is run before SystemB.
			 */
			template<class SystemA, class SystemB>
			constexpr static bool orderBefore();

			/**
			 * Checks if SystemA is run after SystemB.
			 */
			template<class SystemA, class SystemB>
			constexpr static bool orderAfter();

			// TODO: doc
			template<class Callable>
			void callWithComponent(ComponentId cid, Callable&& callable);

			/**
			 * Helper to cast from `this` to CRTP derived class.
			 */
			decltype(auto) self() { return reinterpret_cast<Derived&>(*this); }
			decltype(auto) self() const { return reinterpret_cast<const Derived&>(*this); }

		private:
			void storeSnapshot();
			bool loadSnapshot(Tick tick);
			void tickSystems();

			/**
			 * Get the container for components of type @p Component.
			 * @tparam C The type of the component.
			 * @return A reference to the container associated with @p Component.
			 */
			template<class C>
			ENGINE_INLINE ComponentContainer<C>& getComponentContainer() {
				// Enabling this assert seems to cause compile errors with some of the `if constexpr` stuff
				//static_assert(!IsFlagComponent<C>::value, "Attempting to get the container for a flag component.");
				return std::get<ComponentContainer<C>>(compContainers);
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

			ENGINE_INLINE void destroyMarkedEntities() {
				for (auto ent : markedForDeath) {
					destroyEntity(ent);
				}
				markedForDeath.clear();
			}
	};
}

#include <Engine/ECS/World.ipp>

#undef WORLD_TPARAMS
#undef WORLD_CLASS
