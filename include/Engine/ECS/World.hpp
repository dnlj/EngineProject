#pragma once

// STD
#include <tuple>
#include <type_traits>

// Meta
#include <Meta/IndexOf.hpp>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Clock.hpp>
#include <Engine/ECS/ecs.hpp>
#include <Engine/SequenceBuffer.hpp>
#include <Engine/ECS/EntityFilter.hpp>
#include <Engine/FlatHashMap.hpp>
#include <Engine/Meta/ForEach.hpp>


namespace Engine::ECS {
	// TODO: move
	// TODO: make copy constructor on this and EntityFilter private
	template<bool IncludeDisabled, class C, class World>
	class SingleComponentFilter {
		private:
			using It = decltype(std::declval<World>().template getComponentContainer<C>().begin());
			using CIt = decltype(std::declval<World>().template getComponentContainer<C>().cbegin());
			World& world;

			class Iter {
				private:
					friend class SingleComponentFilter;
					using I = int32;
					It it;
					World& world;

					ENGINE_INLINE static auto& getCont(World& w) { return w.template getComponentContainer<C>(); }
					ENGINE_INLINE static decltype(auto) getContBegin(World& w) { return getCont(w).begin(); }
					ENGINE_INLINE static decltype(auto) getContEnd(World& w) { return getCont(w).end(); }

					// TODO: do we also want to check if enabled? maybe filters need some way to specify what flags an entity should/shouoldnt have
					ENGINE_INLINE bool canUse(Entity ent) const {
						return world.isAlive(ent) && (IncludeDisabled || world.isEnabled(ent));
					}

					ENGINE_INLINE void stepNextValid() {
						const auto end = getContEnd(world);
						while (it != end && !canUse(it->first)) {
							++it;
						}
					}

					ENGINE_INLINE void stepPrevValid() {
						const auto begin = getContBegin(world);
						while (it != begin && !canUse(it->first)) {
							--it;
						}
					}

					static Iter begin(World& w) {
						Iter temp{getContBegin(w), w};
						temp.stepNextValid();
						return temp;
					}

					static Iter end(World& w) {
						Iter temp{getContEnd(w), w};
						return temp;
					}

					Iter(It it, World& world) : it{it}, world{world} {}

				public:
					auto& operator++() {
						const auto end = getContEnd(world);
						if (it != end) { ++it; }
						stepNextValid();
						return *this;
					}

					auto& operator--() {
						const auto begin = getContBegin(world);
						if (it != begin) { --it; }
						stepPrevValid();
						return *this;
					}

					ENGINE_INLINE auto& operator*() {
						ENGINE_DEBUG_ASSERT(canUse(it->first), "Attempt to dereference invalid iterator.");
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

			SingleComponentFilter(const SingleComponentFilter&) = delete;
			SingleComponentFilter& operator=(const SingleComponentFilter&) = delete;

			SingleComponentFilter(SingleComponentFilter&&) = default;
			SingleComponentFilter& operator=(SingleComponentFilter&&) = default;

			ENGINE_INLINE Iter begin() const {
				return Iter::begin(world);
			}

			ENGINE_INLINE Iter end() const {
				return Iter::end(world);
			}

			auto size() const {
				int32 i = 0;
				for (auto it = begin(); it != end(); ++it, ++i) {}
				return i;
			}

			ENGINE_INLINE bool empty() const { return begin() == end(); }
	};

	// TODO: move
	template<class...>
	struct EntityFilterList {};

	template<class T>
	struct IsEntityFilterList : std::false_type {};

	template<class... Cs>
	struct IsEntityFilterList<EntityFilterList<Cs...>> : std::true_type {};

	// TODO: rm - work around for requires clauses not working in `if constexpr` on msvc
	template<class Sys, class Comp>
	concept HasComponentAddedCallbackFor = requires (Sys sys, Engine::ECS::Entity ent, Comp comp) {
		sys.onComponentAdded(ent, comp);
	};

	// TODO: rm - work around for requires clauses not working in `if constexpr` on msvc
	template<class Sys, class Comp>
	concept HasComponentRemovedCallbackFor = requires (Sys sys, Engine::ECS::Entity ent, Comp comp) {
		sys.onComponentRemoved(ent, comp);
	};

	template<class T>
	class SnapshotTraits {
		private:
			using _t_SnapshotTraits_isSpecialized = void;

		public:
			using Type = struct Null{};
			using Container = SparseSet<Entity, Type>;

			static std::tuple<> toSnapshot(const T& obj) {
				static_assert(!sizeof(T), "SnapshotTraits::toSnapshot must be implemented for type T.");
				return std::make_tuple();
			}

			static void fromSnapshot(T& obj, const Type& snap) {
				static_assert(!sizeof(T), "SnapshotTraits::fromSnapshot must be implemented for type T.");
			}
	};

	template<class T>
	concept IsSnapshotRelevant = !requires {
		SnapshotTraits<T>::_t_SnapshotTraits_isSpecialized;
	};
}

namespace Engine::ECS {
	/**
	 * Manages entities, systems, and components.
	 * You should probably use WorldHelper instead.
	 * 
	 * @tparam TickRate The tick rate of the world.
	 * @tparam SystemsSet The systems for this world to have.
	 * @tparam ComponentsSet The components for entities in this world to have.
	 * @tparam FlagsSet The flag components.
	 * @tparam MergedSet A set containing both the components and the flags for this world - Merged<Cs..., Fs...>
	 * @see WorldHelper
	 */
	template<int64 TickRate, class SystemsSet, class ComponentsSet, class FlagsSet, class MergedSet>
	class World;

	#define ECS_WORLD_TPARAMS template<\
		int64 TickRate,\
		class... Ss,\
		template<class...> class SystemsSet,\
		class... Ns_,\
		template<class...> class NonFlagsSet_,\
		class... Fs_,\
		template<class...> class FlagsSet_,\
		class... Cs,\
		template<class...> class ComponentsSet\
	>

	#define ECS_WORLD_CLASS World<TickRate, SystemsSet<Ss...>, NonFlagsSet_<Ns_...>, FlagsSet_<Fs_...>, ComponentsSet<Cs...>>
	
	ECS_WORLD_TPARAMS
	class ECS_WORLD_CLASS {
		static_assert(sizeof...(Cs) <= MAX_COMPONENTS);
		public:
			template<class C> struct IsFlagComponent {
				constexpr static bool value = (std::is_same_v<C, Fs_> || ...);
			};
			template<class C> struct IsNonFlagComponent {
				constexpr static bool value = (std::is_same_v<C, Ns_> || ...);
			};
			template<class C> struct IsAnyComponent {
				constexpr static bool value = IsFlagComponent<C>::value || IsNonFlagComponent<C>::value;
			};
			template<class C> struct ComponentContainer
				: std::conditional_t<IsFlagComponent<C>::value, SparseSet<Entity, void>, SparseSet<Entity, C>> {
			};

			using BaseType = World<TickRate, SystemsSet<Ss...>, NonFlagsSet_<Ns_...>, FlagsSet_<Fs_...>, ComponentsSet<Cs...>>;
			using SystemsSetType = SystemsSet<Ss...>;
			using NonFlagsSetType = NonFlagsSet_<Ns_...>;
			using FlagSetType = FlagsSet_<Fs_...>;
			using ComponentsSetType = ComponentsSet<Cs...>;

		private:
			/** TODO: doc */
			bool performingRollback = false;

			/** Beginning of most recent run. */
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

		private:
			template<bool,class,class>
			friend class SingleComponentFilter;

			std::vector<EntityFilter> filters;
			FlatHashMap<ComponentBitset, int32> cbitsToFilter;
			std::array<std::vector<int32>, sizeof...(Cs)> compToFilter;

			/** Time currently being ticked */
			Clock::TimePoint tickTime = {};

			/** The current tick being run */
			Tick currTick = -1;

			/** The current update  */
			uint64 currUpdate = -1;
				
			/** TODO: doc */
			EntityStates entities;
				
			/** TODO: doc */
			std::vector<Entity> deadEntities;

			/** TODO: doc */
			std::vector<Entity> markedForDeath;

			/** The bitsets for storing what components entities have. */
			std::vector<ComponentBitset> compBitsets;

			/** The containers for storing components. */
			void* compContainers[sizeof...(Cs)] = {};

			struct {
				Engine::ECS::Tick tick = -1;
				Clock::TimePoint time = {};
			} rollbackData;

			class Snapshot {
				public:
					Snapshot();
					Snapshot(const Snapshot&) = delete;
					Snapshot& operator=(const Snapshot&) = delete;
					~Snapshot();

					Clock::TimePoint tickTime = {};
					void* compContainers[sizeof...(Cs)] = {};

					template<class C>
					ENGINE_INLINE auto& getComponentContainer() {
						static_assert(IsSnapshotRelevant<C>,
							"Attempting to get component container for non-snapshot relevant component."
						);
						ENGINE_DEBUG_ASSERT(getComponentContainer_Unsafe<C>() != nullptr);
						return *getComponentContainer_Unsafe<C>();
					}

					template<class C>
					ENGINE_INLINE const auto& getComponentContainer() const { return const_cast<Snapshot*>(this)->getComponentContainer<C>(); }

				private:
					template<class C>
					ENGINE_INLINE auto* getComponentContainer_Unsafe() {
						return reinterpret_cast<SnapshotTraits<C>::Container*>(compContainers[getComponentId<C>()]);
					}

			};

			SequenceBuffer<Tick, Snapshot, TickRate> history;
			
			/** All the systems in this world. */
			void* systems[sizeof...(Ss)] = {};
			
		public:
			// TODO: doc
			template<class Arg>
			World(Arg&& arg);
			World(const World&) = delete;
			~World();

			/**
			 * Advances simulation time and calls the `tick` and `update` members of systems.
			 */
			void run();

			ENGINE_INLINE bool isPerformingRollback() const noexcept { return performingRollback; }
			ENGINE_INLINE void scheduleRollback(Tick t) { rollbackData.tick = t; }
			ENGINE_INLINE bool hasHistory(Tick tick) const { return history.contains(tick); }
			
			////////////////////////////////////////////////////////////////////////////////
			// Entity Functions
			////////////////////////////////////////////////////////////////////////////////
			// TODO: Doc. valid vs current vs alive vs enabled
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
			 * Until the Entity is destroyed it is disabled.
			 */
			ENGINE_INLINE void deferedDestroyEntity(Entity ent) {
				setEnabled(ent, false); // TODO: Will we need a component callback for onDisabled to handle things like physics bodies?
				// TODO: would it be better to sort the list afterward (in World::storeSnapshot for example)? instead of while inserting
				markedForDeath.insert(std::lower_bound(markedForDeath.cbegin(), markedForDeath.cend(), ent), ent);
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
			ENGINE_INLINE constexpr static SystemId getSystemId() noexcept { return ::Meta::IndexOf<System, Ss...>::value; }

			/**
			 * Gets the instance of the system for this world.
			 */
			template<class System>
			System& getSystem(){
				return *reinterpret_cast<System*>(systems[getSystemId<System>()]);
			}
			/**
			 * Gets a bitset with the bits for the given systems set.
			 */
			template<class... SystemN>
			SystemBitset getBitsetForSystems() const { // TODO: constexpr, noexcept
				SystemBitset value;
				((value[getSystemId<SystemN>()] = true), ...);
				return value;
			}

			ENGINE_INLINE void debugEntityCheck(Entity e) const {
				ENGINE_DEBUG_ASSERT(isValid(e), "Attempting to use invalid entity");
				ENGINE_DEBUG_ASSERT(isAlive(e), "Attempting to use old entity");
			}

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
				static_assert(IsAnyComponent<Component>::value, "Attempting to get component id of type that is not in the component list. Did you forget to add it?");
				return ::Meta::IndexOf<Component, Cs...>::value;
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
			decltype(auto) addComponent(Entity ent, Args&&... args) {
				debugEntityCheck(ent);
				constexpr auto cid = getComponentId<C>();
				ENGINE_DEBUG_ASSERT(!hasComponent<C>(ent), "Attempting to add duplicate component (", cid ,") to ", ent);
				auto& cbits = compBitsets[ent.id];
				cbits.set(cid);
				
				auto& container = getComponentContainer<C>();
				auto& comp = container.add(ent, std::forward<Args>(args)...);

				// Update filters
				for (const auto i : compToFilter[cid]) {
					auto& filter = filters[i];
					filter.add(ent, cbits);
					// ENGINE_INFO("Adding ", ent, " to filter ", i, " ( C = ", getComponentId<C>(), ")");
				}

				Meta::ForEach<Ss...>::call([&]<class S>() ENGINE_INLINE {
					if constexpr (HasComponentAddedCallbackFor<S, C>) {
						getSystem<S>().onComponentAdded(ent, comp);
					}
				});

				return comp;
			}

			/**
			 * Adds components to an entity.
			 * @param[in] ent The entity.
			 * @tparam Components The components.
			 * @return A tuple of references to the added components.
			 */
			template<class... Components>
			ENGINE_INLINE decltype(auto) addComponents(Entity ent) { debugEntityCheck(ent); return std::forward_as_tuple(addComponent<Components>(ent) ...); };

			/**
			 * Checks if an entity has a component.
			 * @param[in] ent The entity.
			 * @param[in] cid The id of the component.
			 * @return True if the entity has the component; otherwise false.
			 */
			// TODO: use getComponentsBitset
			ENGINE_INLINE bool hasComponent(Entity ent, ComponentId cid) const { debugEntityCheck(ent); return compBitsets[ent.id].test(cid); }

			/**
			 * Checks if an entity has a component.
			 * @param[in] ent The entity.
			 * @tparam Component The component type.
			 * @return True if the entity has the component; otherwise false.
			 */
			template<class C>
			ENGINE_INLINE bool hasComponent(Entity ent) const { debugEntityCheck(ent); return hasComponent(ent, getComponentId<C>()); }
			
			/**
			 * Removes a component from an entity.
			 * @param[in] ent The entity.
			 * @tparam C The component.
			 */
			template<class C>
			ENGINE_INLINE void removeComponent(Entity ent) {
				debugEntityCheck(ent);
				C* comp = nullptr;

				if constexpr (!IsFlagComponent<C>::value) {
					comp = &getComponent<C>(ent);
				}

				// Callbacks
				Meta::ForEach<Ss...>::call([&]<class S>() ENGINE_INLINE {
					if constexpr (HasComponentRemovedCallbackFor<S, C>) {
						getSystem<S>().onComponentRemoved(ent, *comp);
					}
				});

				// Remove
				compBitsets[ent.id] &= ~getBitsetForComponents<C>();
				getComponentContainer<C>().erase(ent);

				// Update Filters
				auto& is = compToFilter[getComponentId<C>()];
				for (auto i : is) {
					filters[i].remove(ent);
				}
			};

			/**
			 * Removes all components from an entity.
			 */
			ENGINE_INLINE void removeAllComponents(Entity ent) {
				debugEntityCheck(ent);
				((hasComponent<Cs>(ent) && (removeComponent<Cs>(ent), 1)), ...);
			};

			/**
			 * Gets a reference to the component instance associated with an entity.
			 */
			template<class Component>
			ENGINE_INLINE Component& getComponent(Entity ent) {
				debugEntityCheck(ent);
				// TODO: why is this not a compile error? this should need `decltype(auto)` return type?
				static_assert(!IsFlagComponent<Component>::value,
					"Calling World::getComponent on a flag component is not allowed. Use World::hasComponent instead."
				);

				ENGINE_DEBUG_ASSERT(hasComponent<Component>(ent), "Attempting to get a component that an entity doesn't have.");
				return getComponentContainer<Component>()[ent];
			}

			template<class Component>
			ENGINE_INLINE const Component& getComponent(Entity ent) const {
				debugEntityCheck(ent);
				return const_cast<World*>(this)->getComponent<Component>(ent);
			}

			template<class C>
			ENGINE_INLINE bool hadComponent(Entity ent, Tick tick) const {
				return history.get(tick).getComponentContainer<C>().contains(ent);
			}

			// TODO: Doc
			/**
			 * 
			 */
			template<class C>
			ENGINE_INLINE auto& getComponentState(Entity ent, Tick tick) {
				static_assert(IsSnapshotRelevant<C>,
					"Attempting to get component from snapshot for non-snapshot relevant component."
				);

				auto& snap = history.get(tick);
				auto& cont = snap.getComponentContainer<C>();
				if (!cont.contains(ent)) {
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
				debugEntityCheck(ent);
				return std::forward_as_tuple(getComponent<Components>(ent) ...);
			}

			template<class... Components>
			ENGINE_INLINE std::tuple<const Components&...> getComponents(Entity ent) const {
				debugEntityCheck(ent);
				return std::forward_as_tuple(getComponent<Components>(ent) ...);
			}

			/**
			 * Gets the components bitset for an entity.
			 * @param[in] ent The entity.
			 * @return The components bitset for the entity
			 */
			ENGINE_INLINE decltype(auto) getComponentsBitset(Entity ent) const noexcept { debugEntityCheck(ent); return compBitsets[ent.id]; }

			/**
			 * Gets the components bitset for all entities. Sorted by entity id. 
			 */
			ENGINE_INLINE const auto& getAllComponentBitsets() const { return compBitsets; };

			// TODO: doc
			template<bool IncludeDisabled, class C, class... Comps>
			decltype(auto) getFilterAll() {
				if constexpr (IsEntityFilterList<C>::value) {
					static_assert(sizeof...(Comps) == 0);
					return [&]<class... Ds>(EntityFilterList<Ds...>) -> decltype(auto) {
						return getFilterAll<IncludeDisabled, Ds...>();
					}(C{});
				} else if constexpr (sizeof...(Comps) == 0) {
					return SingleComponentFilter<IncludeDisabled, C, World>{*this};
				} else {
					const auto cbits = getBitsetForComponents<C, Comps...>();
					auto found = cbitsToFilter.find(cbits);

					// TODO: maybe having EntityFilter be more of a "view" class would be better to avoid this `.with` stuff and the accidental copy concern.
					if (found != cbitsToFilter.end()) {
						// TODO: why do we have this `with` function? i assume at one point
						// TODO: cont. it was for handling rollback but atm it doesnt look like it ever changes.
						return filters[found->second].with(entities, IncludeDisabled);
					}

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

			template<class C, class... Comps>
			ENGINE_INLINE decltype(auto) getFilter() {
				return getFilterAll<false, C, Comps...>();
			}
			
			/**
			 * Gets the current tick.
			 */
			ENGINE_INLINE auto getTick() const noexcept { return currTick; }

			/**
			 * Gets the current update.
			 */
			ENGINE_INLINE auto getUpdate() const noexcept { return currUpdate; }

			// TODO: doc
			void setNextTick(Tick tick) {
				// TODO: defer this till next `run`
				currTick = tick - 1;
				tickTime = Clock::now();
				history.clear(tick);
			}

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
			 * The time at the start of the current update.
			 */
			ENGINE_INLINE Clock::TimePoint getTime() const noexcept { return beginTime; };

			/**
			 * Gets the time (in seconds) last update took to run.
			 */
			ENGINE_INLINE float32 getDeltaTime() const noexcept { return deltaTime; }

			/**
			 * Gets the time (in nanoseconds) last update took to run.
			 */
			ENGINE_INLINE auto getDeltaTimeNS() const noexcept { return deltaTimeNS; }

			/**
			 * Checks if SystemA is run before SystemB.
			 */
			template<class SystemA, class SystemB>
			constexpr static bool orderBefore() noexcept { return ::Meta::IndexOf<SystemA, Ss...>::value < ::Meta::IndexOf<SystemB, Ss...>::value; }

			/**
			 * Checks if SystemA is run after SystemB.
			 */
			template<class SystemA, class SystemB>
			constexpr static bool orderAfter() noexcept { return orderBefore<SystemB, SystemA>(); }

			// TODO: doc
			template<class Callable>
			void callWithComponent(ComponentId cid, Callable&& callable) {
				using Caller = void(Callable::*)(void) const;
				constexpr Caller callers[]{ &Callable::template operator()<Cs>... };
				return (callable.*callers[cid])();
			}

		private:
			void storeSnapshot();
			bool loadSnapshot(Tick tick);
			void tickSystems();
			void updateSystems();

			/**
			 * Get the container for components of type @p Component.
			 * @tparam C The type of the component.
			 * @return A reference to the container associated with @p Component.
			 */
			template<class C>
			ENGINE_INLINE ComponentContainer<C>& getComponentContainer() {
				return *static_cast<ComponentContainer<C>*>(compContainers[getComponentId<C>()]);
			}

			/**
			 * Destroys and entity, freeing its id to be recycled.
			 */
			void destroyEntity(Entity ent) {
				removeAllComponents(ent);
		
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

	/**
	 * Helper class to automatically build the merged set for a World.
	 * @see World
	 */
	template<int64 TickRate, class SystemsSet, class ComponentsSet, class FlagsSet>
	class WorldHelper;

	template<
		int64 TickRate,
		class... Ss, template<class...> class SystemsSet,
		class... Cs, template<class...> class ComponentsSet,
		class... Fs, template<class...> class FlagsSet
	>
	class WorldHelper<TickRate, SystemsSet<Ss...>, ComponentsSet<Cs...>, FlagsSet<Fs...>>
		: public World<TickRate, SystemsSet<Ss...>, ComponentsSet<Cs...>, FlagsSet<Fs...>, std::tuple<Cs..., Fs...>> {
		public:
			using World<TickRate, SystemsSet<Ss...>, ComponentsSet<Cs...>, FlagsSet<Fs...>, std::tuple<Cs..., Fs...>>::World;
	};
}
