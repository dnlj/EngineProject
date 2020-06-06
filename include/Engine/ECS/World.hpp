#pragma once

// STD
#include <tuple>
#include <type_traits>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Clock.hpp>
#include <Engine/ECS/Common.hpp>
#include <Engine/ECS/FilterManager.hpp>


namespace Engine::ECS {
	/**
	 * @tparam Derived CRTP dervied class. Needed for EntityFilter<Derived>.
	 * @tparam TickRate The tick rate of the world.
	 * @tparam SystemsSet The systems for this world to have.
	 * @tparam ComponentsSet The components for entities in this world to have.
	 * @tparam FlagsSet The flags for entities in this world to have.
	 */
	template<class Derived, int64 TickRate, class SystemsSet, class ComponentsSet, class FlagsSet>
	class World;

	#define WORLD_TPARAMS template<\
		class Derived,\
		int64 TickRate,\
		class... Ss,\
		template<class...> class SystemsSet,\
		class... Cs,\
		template<class...> class ComponentsSet,\
		class... Fs,\
		template<class...> class FlagsSet\
	>

	#define WORLD_CLASS World<Derived, TickRate, SystemsSet<Ss...>, ComponentsSet<Cs...>, FlagsSet<Fs...>>
	
	class EntityState {
		public:
			enum State : uint8 {
				Dead = 0 << 0,
				Alive =  1 << 0,
				Enabled = 1 << 1,
			};

			EntityState(Entity ent, State state) : ent{ent}, state{state} {}
			Entity ent;
			uint8 state;
	};

	WORLD_TPARAMS
	class WORLD_CLASS {
		static_assert(sizeof...(Cs) + sizeof...(Fs) <= MAX_COMPONENTS);
		public:
			using Filter = EntityFilter<Derived>;

		private:
			// TODO: Since we are wrapping all of these operations is there any real benefit to splitting into XYZManagers?
			FilterManager<Derived> fm;

			/** Beginning of last run. */
			Clock::TimePoint beginTime;

			/** Time currently being ticked */
			Clock::TimePoint tickTime;

			/** Maximum tick delay to accumulate */
			constexpr static Clock::Duration maxDelay = std::chrono::milliseconds{250};

			/** How long between each tick. */
			constexpr static Clock::Duration tickInterval{Clock::Period::den / TickRate};
			static_assert(tickInterval < maxDelay, "Tick interval must be less than the maximum accumulable tick duration.");
			
			/** Time it took to process the last run. */
			float32 deltaTime = 0.0f;
			
			/** Delta time in nanoseconds. @see deltaTime */
			Clock::Duration deltaTimeNS{0};

			/** The containers for storing components. */
			std::tuple<ComponentContainer<Cs>...> compContainers;

			/** The bitsets for storing what components entities have. */
			std::vector<ComponentBitset> compBitsets;

			/** All the systems in this world. */
			std::tuple<Ss...> systems;

			/** TODO: doc */
			std::vector<Entity> deadEntities;

			/** TODo: doc */
			std::vector<EntityState> entities;

		public:
			// TODO: doc
			template<class Arg>
			World(float tickInterval, Arg& arg);

			World(const World&) = delete;

			/**
			 * Advances simulation time and calls the `tick` and `run` members of systems.
			 */
			void run();

			/**
			 * Checks if an entity is alive.
			 */
			bool isAlive(Entity ent) const;

			/**
			 * Enables or disables and entity.
			 */
			void setEnabled(Entity ent, bool enabled);

			/**
			 * Checks if an entity is enabled.
			 */
			bool isEnabled(Entity ent) const;

			/**
			 * Gets all entities.
			 */
			auto& getEntities() const;
			
			/**
			 * Gets the bitset with the bits that correspond to the ids of the given components set.
			 */
			template<class... ComponentN>
			ComponentBitset getBitsetForComponents() const; // TODO: constexpr, noexcept
			
			/**
			 * Get the ComponentId associated with a component.
			 * @tparam Component The component.
			 * @return The id of @p Component.
			 */
			template<class Component>
			constexpr static ComponentId getComponentId() noexcept;

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
			Entity createEntity(bool forceNew = false);

			/**
			 * Destroys and entity, freeing its id to be recycled.
			 */
			void destroyEntity(Entity ent);
			
			/**
			 * Checks if a type is a flag.
			 */
			template<class F>
			constexpr static bool isFlag();

			/**
			 * Checks if a type is a component.
			 */
			template<class C>
			constexpr static bool isComponent();

			/**
			 * Adds a component to an entity.
			 * @param ent The entity.
			 * @param args The arguments to pass to the constructor of the component.
			 * @tparam Component The component.
			 * @return A reference to the added component.
			 */
			template<class Component, class... Args>
			Component& addComponent(Entity ent, Args&&... args);

			/**
			 * Adds components to an entity.
			 * @param[in] ent The entity.
			 * @tparam Components The components.
			 * @return A tuple of references to the added components.
			 */
			template<class... Components>
			std::tuple<Components&...> addComponents(Entity ent);

			/**
			 * Checks if an entity has a component.
			 * @param[in] ent The entity.
			 * @param[in] cid The id of the component.
			 * @return True if the entity has the component; otherwise false.
			 */
			bool hasComponent(Entity ent, ComponentId cid);

			/**
			 * Checks if an entity has a component.
			 * @param[in] ent The entity.
			 * @tparam Component The component type.
			 * @return True if the entity has the component; otherwise false.
			 */
			template<class Component>
			bool hasComponent(Entity ent);

			/**
			 * Checks if an entity has components.
			 * @param[in] ent The entity.
			 * @param[in] cbits The bitset of components.
			 * @return True if the entity has the components; otherwise false.
			 */
			bool hasComponents(Entity ent, ComponentBitset cbits);

			/**
			 * Checks if an entity has the components.
			 * @param[in] ent The entity.
			 * @tparam Components The components.
			 * @return True if the entity has the components; otherwise false.
			 */
			template<class... Components>
			bool hasComponents(Entity ent);

			/**
			 * Removes a component from an entity.
			 * @param[in] ent The entity.
			 * @tparam Component The component.
			 */
			template<class Component>
			void removeComponent(Entity ent);

			/**
			 * Removes components from an entity.
			 * @param[in] ent The entity.
			 * @tparam Components The components.
			 */
			template<class... Components>
			void removeComponents(Entity ent);

			/**
			 * Removes all components from an entity.
			 */
			void removeAllComponents(Entity ent);

			/**
			 * Gets a reference to the component instance associated with an entity.
			 */
			template<class Component>
			Component& getComponent(Entity ent);

			/**
			 * Gets a reference the components associated with an entity.
			 * @param[in] ent The entity.
			 * @tparam Components The components.
			 * @return A tuple of references to the components.
			 */
			template<class... Components>
			std::tuple<Components&...> getComponents(Entity ent);

			/**
			 * Gets the components bitset for an entity.
			 * @param[in] ent The entity.
			 * @return The components bitset for the entity
			 */
			const ComponentBitset& getComponentsBitset(Entity ent) const;

			// TODO: Doc
			template<class... Components>
			Filter& getFilterFor();

			//template<>
			//EntityFilter& getFilterFor() = delete;
			
			/**
			 * Gets the tick interval.
			 */
			auto getTickInterval() const;

			/**
			 * Current time being ticked.
			 */
			Clock::TimePoint getTickTime() const;
			
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
			void callWithComponent(Entity ent, ComponentId cid, Callable&& callable);

			/**
			 * Helper to cast from `this` to CRTP derived class.
			 */
			decltype(auto) self() { return reinterpret_cast<Derived&>(*this); }
			decltype(auto) self() const { return reinterpret_cast<const Derived&>(*this); }

		private:
			/**
			 * Get the container for components of type @p Component.
			 * @tparam Component The type of the component.
			 * @return A reference to the container associated with @p Component.
			 */
			template<class Component>
			ComponentContainer<Component>& getComponentContainer();
	};
}

#include <Engine/ECS/World.ipp>

#undef WORLD_TPARAMS
#undef WORLD_CLASS
