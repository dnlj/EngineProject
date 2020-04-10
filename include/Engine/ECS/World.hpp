#pragma once

// STD
#include <tuple>
#include <type_traits>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Clock.hpp>
#include <Engine/ECS/Common.hpp>
#include <Engine/ECS/SystemManager.hpp>
#include <Engine/ECS/ComponentManager.hpp>
#include <Engine/ECS/EntityManager.hpp>
#include <Engine/ECS/FilterManager.hpp>


namespace Engine::ECS {
	/**
	 * @tparam TickRate The tick rate of the world.
	 * @tparam SystemsSet The systems for this world to have.
	 * @tparam ComponentsSet The components for this world to have.
	 */
	template<int64 TickRate, class SystemsSet, class ComponentsSet>
	class World {
		public:
			// TODO: Since we are wrapping all of these operations is there any real benifit to splitting into XYZManagers?
			using ComponentManager = ComponentManager<ComponentsSet>;
			using SystemManager = SystemManager<SystemsSet>;

		private:
			EntityManager em;
			ComponentManager cm;
			FilterManager fm;
			SystemManager sm;

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


		public:
			/**
			 * @see SystemManager::SystemManager
			 */
			template<class Arg>
			World(float tickInterval, Arg& arg);

			// TODO: doc
			void run();

			/**
			 * @see EntityManager::isAlive
			 */
			bool isAlive(Entity ent) const;

			/**
			 * @see EntityManager::setEnabled
			 */
			void setEnabled(Entity ent, bool enabled);

			/**
			 * @see EntityManager::isEnabled
			 */
			bool isEnabled(Entity ent) const;

			/**
			 * @see EntityManager::getEntities
			 */
			const EntityManager::EntityContainer& getEntities() const;

			/**
			 * @see ComponentManager<T>::getBitsetForComponents
			 */
			template<class... ComponentN>
			ComponentBitset getBitsetForComponents() const;
			
			/**
			 * @see ComponentManager::getComponentId
			 */
			template<class Component>
			constexpr static ComponentId getComponentId() noexcept;

			/**
			 * @see SystemManager::getSystemID
			 */
			template<class System>
			constexpr static SystemID getSystemID() noexcept;

			/**
			 * @see SystemManager::getSystem
			 */
			template<class System>
			System& getSystem();

			/**
			 * @see SystemManager::getBitsetForSystems
			 */
			template<class... SystemN>
			SystemBitset getBitsetForSystems() const;

			/**
			 * @see SystemManager::run
			 */
			void run(float dt);

			/**
			 * @see EntityManager::createEntity
			 */
			Entity createEntity(bool forceNew = false);

			/**
			 * @see EntityManager::destroyEntity
			 */
			void destroyEntity(Entity ent);

			/**
			 * Adds a component to an entity.
			 * @param[in] ent The entity.
			 * @tparam Component The component.
			 * @return A reference to the added component.
			 */
			template<class Component>
			Component& addComponent(Entity ent);

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
			 * Gets a reference the component associated with an entity.
			 * @param[in] ent The entity.
			 * @tparam Component The component.
			 * @return A reference to the component.
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
			ComponentBitset getComponentsBitset(Entity ent) const;

			// TODO: Doc
			template<class... Components>
			EntityFilter& getFilterFor();

			template<>
			EntityFilter& getFilterFor() = delete;
			
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

			/** @see SystemManager::orderBefore */
			template<class SystemA, class SystemB>
			constexpr static bool orderBefore();

			/** @see SystemManager::orderAfter */
			template<class SystemA, class SystemB>
			constexpr static bool orderAfter();

			// TODO: doc
			template<class Callable>
			void callWithComponent(Entity ent, ComponentId cid, Callable&& callable);

	};
}

#include <Engine/ECS/World.ipp>
