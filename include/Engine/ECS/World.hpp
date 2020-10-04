#pragma once

// STD
#include <tuple>
#include <type_traits>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Clock.hpp>
#include <Engine/ECS/Common.hpp>
#include <Engine/ECS/FilterManager.hpp>
#include <Engine/ECS/Snapshot.hpp>
#include <Engine/SequenceBuffer.hpp>


namespace Engine::ECS {
	/**
	 * @tparam Derived CRTP dervied class. Needed for EntityFilter<Derived>.
	 * @tparam TickRate The tick rate of the world.
	 * @tparam SnapshotCount The number of snapshots to keep for rollback.
	 * @tparam SystemsSet The systems for this world to have.
	 * @tparam ComponentsSet The components for entities in this world to have.
	 */
	template<class Derived, int64 TickRate, int64 SnapshotCount, class SystemsSet, class ComponentsSet>
	class World;

	#define WORLD_TPARAMS template<\
		class Derived,\
		int64 TickRate,\
		int64 SnapshotCount,\
		class... Ss,\
		template<class...> class SystemsSet,\
		class... Cs,\
		template<class...> class ComponentsSet\
	>

	#define WORLD_CLASS World<Derived, TickRate, SnapshotCount, SystemsSet<Ss...>, ComponentsSet<Cs...>>
	
	// TODO: move
	template<class T, class = void>
	struct IsSnapshotRelevant : std::false_type {};

	template<class T>
	struct IsSnapshotRelevant<T, std::enable_if_t<T::isSnapshotRelevant>> : std::true_type {};

	WORLD_TPARAMS
	class WORLD_CLASS {
		static_assert(sizeof...(Cs) <= MAX_COMPONENTS);
		public:
			using Filter = EntityFilter<Derived>;

		private:
			// TODO: Since we are wrapping all of these operations is there any real benefit to splitting into XYZManagers?
			FilterManager<Derived> fm;

			/** Beginning of last run. */
			Clock::TimePoint beginTime;

			/** Maximum tick delay to accumulate */
			constexpr static Clock::Duration maxDelay = std::chrono::milliseconds{250};

			// TODO: shouldnt this be part of snapshot? that would fix our issue with correct tickTime i think
			// TODO: not public
			/** TODO: doc */
			public: float32 tickScale = 1.0f; private:

			/** How long between each tick. */
			constexpr static Clock::Duration tickInterval{Clock::Period::den / TickRate};
			static_assert(tickInterval < maxDelay, "Tick interval must be less than the maximum accumulable tick duration.");

			/** The tick interval in floating point seconds. */
			constexpr static float32 tickDeltaTime = Clock::Seconds{tickInterval}.count();
			
			/** Time it took to process the last run. */
			float32 deltaTime = 0.0f;
			
			/** Delta time in nanoseconds. @see deltaTime */
			Clock::Duration deltaTimeNS{0};

			/** All the systems in this world. */
			std::tuple<Ss...> systems;

			/** TODO: doc */
			bool performingRollback = false;

			template<class T> struct AlwaysTrue : std::true_type {};
			using ActiveSnapshot = Snapshot<AlwaysTrue, Cs...>;
			using RollbackSnapshot = Snapshot<IsSnapshotRelevant, Cs...>;
			ActiveSnapshot activeSnap;
			SequenceBuffer<Tick, RollbackSnapshot, SnapshotCount> snapBuffer;

		public:
			// TODO: doc
			template<class Arg>
			World(Arg& arg);

			World(const World&) = delete;

			/**
			 * Advances simulation time and calls the `tick` and `run` members of systems.
			 */
			void run();

			/**
			 * Checks if an entity is alive.
			 */
			ENGINE_INLINE bool isAlive(Entity ent) const noexcept { return activeSnap.isEnabled(ent); };

			/**
			 * Enables or disables and entity.
			 */
			ENGINE_INLINE void setEnabled(Entity ent, bool enabled) { return activeSnap.setEnabled(ent, enabled); };

			/**
			 * Checks if an entity is enabled.
			 */
			ENGINE_INLINE bool isEnabled(Entity ent) const noexcept { return activeSnap.isEnabled(ent); };

			/**
			 * Checks if an entity should be networked.
			 */
			ENGINE_INLINE bool isNetworked(Entity ent) const noexcept { return activeSnap.isNetworked(ent); };
			
			/**
			 * Enables or disables entity networking.
			 */
			ENGINE_INLINE void setNetworked(Entity ent, bool enabled) noexcept { return activeSnap.setNetworked(ent, enabled); };

			/**
			 * Gets all entities.
			 */
			ENGINE_INLINE auto& getEntities() const noexcept { return activeSnap.getEntities(); };
			
			/**
			 * Get the ComponentId associated with a component.
			 * @tparam Component The component.
			 * @return The id of @p C.
			 */
			template<class C>
			constexpr static auto getComponentId() noexcept { return ActiveSnapshot::getComponentId<C>(); };

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

			/**
			 * Creates an entity.
			 * @param forceNew Disables recycling entity ids.
			 */
			Entity createEntity(bool forceNew = false) { return activeSnap.createEntity(forceNew); };
			
			/**
			 * Marks an Entity to be destroyed once it is out of rollback scope.
			 * Until the Enttiy is destroyed it is disabled.
			 */
			void deferedDestroyEntity(Entity ent) { activeSnap.deferedDestroyEntity(ent); };

			/**
			 * Adds a component to an entity.
			 * @param ent The entity.
			 * @param args The arguments to pass to the constructor of the component.
			 * @tparam C The component.
			 * @return A reference to the added component.
			 */
			template<class C, class... Args>
			decltype(auto) addComponent(Entity ent, Args&&... args) {
				auto& c = activeSnap.addComponent<C>(ent, args...);
				fm.onComponentAdded(ent, getComponentId<C>(), activeSnap.compBitsets[ent.id]);
				return c;
			}

			/**
			 * Adds components to an entity.
			 * @param[in] ent The entity.
			 * @tparam Components The components.
			 * @return A tuple of references to the added components.
			 */
			template<class... Components>
			decltype(auto) addComponents(Entity ent) { return std::forward_as_tuple(addComponent<Components>(ent) ...); };

			/**
			 * Checks if an entity has a component.
			 * @param[in] ent The entity.
			 * @param[in] cid The id of the component.
			 * @return True if the entity has the component; otherwise false.
			 */
			bool hasComponent(Entity ent, ComponentId cid) { return activeSnape.hasComponent(ent, cid); };

			/**
			 * Checks if an entity has a component.
			 * @param[in] ent The entity.
			 * @tparam Component The component type.
			 * @return True if the entity has the component; otherwise false.
			 */
			template<class C>
			bool hasComponent(Entity ent) { return activeSnap.hasComponent<C>(ent); };

			/**
			 * Removes a component from an entity.
			 * @param[in] ent The entity.
			 * @tparam C The component.
			 */
			template<class C>
			void removeComponent(Entity ent) { removeComponents<C>(ent); };

			/**
			 * Removes components from an entity.
			 * @param[in] ent The entity.
			 * @tparam Components The components.
			 */
			template<class... Components>
			void removeComponents(Entity ent) {
				activeSnap.removeComponents<Components...>(ent);
				(fm.onComponentRemoved(ent, getComponentId<Components>()), ...);
			};

			/**
			 * Removes all components from an entity.
			 */
			void removeAllComponents(Entity ent) { ((hasComponent<Cs>(ent) && (removeComponent<Cs>(ent), 1)), ...); };

			/**
			 * Gets a reference to the component instance associated with an entity.
			 */
			template<class C>
			decltype(auto) getComponent(Entity ent) { return activeSnap.getComponent<C>(ent); };

			/**
			 * Gets a reference the components associated with an entity.
			 * @param[in] ent The entity.
			 * @tparam Components The components.
			 * @return A tuple of references to the components.
			 */
			template<class... Components>
			decltype(auto) getComponents(Entity ent) { return getComponents<Components...>(ent); };

			/**
			 * Gets the components bitset for an entity.
			 * @param[in] ent The entity.
			 * @return The components bitset for the entity
			 */
			decltype(auto) getComponentsBitset(Entity ent) const noexcept { return activeSnap.getComponentsBitset(ent); }

			/**
			 * Gets the components bitset for all entities. Sorted by entity id. 
			 */
			const auto& getAllComponentBitsets() const { return activeSnap.compBitsets; };

			// TODO: Doc
			template<class... Components>
			Filter& getFilterFor();

			//template<>
			//EntityFilter& getFilterFor() = delete;
			
			/**
			 * Gets the current tick.
			 */
			auto getTick() const { return activeSnap.currTick; }

			// TODO: also need to clear rollback history
			// TODO: should this be setNextTick? might help avoid bugs if we wait till all systems are done before we adjust.
			// TODO: doc
			void setTick(Tick tick) { activeSnap.currTick = tick; }

			/**
			 * Gets the tick interval.
			 * @see tickInterval
			 */
			constexpr static auto getTickInterval() { return tickInterval; };

			/**
			 * Gets the tick delta.
			 */
			constexpr static auto getTickDelta() { return tickDeltaTime; }

			/**
			 * Current time being ticked.
			 */
			ENGINE_INLINE Clock::TimePoint getTickTime() const noexcept { return activeSnap.tickTime; };
			
			/**
			 * Gets tick accumulation divided by tick interval.
			 * Useful for interpolation.
			 */
			float32 getTickRatio() const;

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

			ENGINE_INLINE bool isPerformingRollback() const noexcept { return performingRollback; }

			constexpr auto getSnapshotCount() { return SnapshotCount; }

		private:
			void tickSystems();

			/**
			 * Destroys and entity, freeing its id to be recycled.
			 */
			void destroyEntity(Entity ent);

			//void storeSnapshot();
			//void loadSnapshot(const RollbackSnapshot& snap);
	};
}

#include <Engine/ECS/World.ipp>

#undef WORLD_TPARAMS
#undef WORLD_CLASS
