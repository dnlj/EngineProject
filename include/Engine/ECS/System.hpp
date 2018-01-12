#pragma once

// Engine
#include <Engine/ECS/ECS.hpp>

#define ENGINE_REGISTER_SYSTEM(s) \
	namespace {\
		const auto __ENGINE_ECS_SYSTEM_DEF__ ## s ## __ = Engine::ECS::detail::registerSystem<s>();\
	}

namespace Engine::ECS {
	// TODO: Doc
	template<class System1, class System2, class... Systems>
	SystemBitset getBitsetForSystems();

	// TODO: Doc
	template<class System>
	SystemBitset getBitsetForSystems();

	/**
	 * @brief Used to check if a type is valid for use as a system.
	 * @tparam T The type to check.
	 */
	template<class T, class = void>
	class IsSystem : public std::false_type {};

	/** @copydoc IsSystem */
	template<class T>
	class IsSystem<
		T,
		std::void_t<
			decltype(
				std::declval<T>().onEntityCreated(std::declval<Engine::Entity>()),
				std::declval<T>().onComponentAdded(std::declval<Engine::Entity>(), std::declval<ComponentID>()),
				std::declval<T>().onComponentRemoved(std::declval<Engine::Entity>(), std::declval<ComponentID>()),
				std::declval<T>().onEntityDestroyed(std::declval<Engine::Entity>()),
				std::declval<T>().run(std::declval<float>()),
				std::declval<T>().priorityBefore,
				std::declval<T>().priorityAfter
			)
		>
	> : public std::true_type{};
}

namespace Engine::ECS::detail {
	class PriorityPair {
		public:
			PriorityPair() = default;
			PriorityPair(SystemBitset priorityBefore, SystemBitset priorityAfter);
			SystemBitset priorityBefore;
			SystemBitset priorityAfter;
	};

	/**
	 * @brief Stores data about the registered systems.
	 */
	namespace SystemData {
		using EntityModifyFunction = void(*)(EntityID);
		using ComponentModifyFunction = void(*)(EntityID, ComponentID);
		using RunFunction = void(*)(float);

		extern std::vector<EntityModifyFunction> onEntityCreated;
		extern std::vector<ComponentModifyFunction> onComponentAdded;
		extern std::vector<ComponentModifyFunction> onComponentRemoved;
		extern std::vector<EntityModifyFunction> onEntityDestroyed;
		extern std::vector<RunFunction> run;
		extern std::array<SystemBitset, MAX_SYSTEMS> priority;
	}

	/**
	 * @brief Gets the a reference to the system.
	 * @tparam System The type of the system.
	 * @return A reference to the system.
	 */
	template<class System>
	System& getSystem();

	/**
	 * @brief Gets the next SystemID.
	 * @return The next SystemID.
	 */
	SystemID getNextSystemID();

	/**
	 * @brief Get the SystemID associated an system.
	 * @tparam System The system.
	 * @return The id of @p System.
	 */
	template<class System>
	SystemID getSystemID();

	/**
	 * @brief Calls the onEntityCreated member function on the system.
	 * @tparam System The type of the system.
	 * @param[in] eid The id of the entity being created.
	 */
	template<class System>
	void onEntityCreated(EntityID eid);

	/**
	 * @brief Calls the onComponentAdded member function on the system.
	 * @tparam System The type of the system.
	 * @param[in] eid The id of the entity being added to.
	 * @param[in] cid The id of the component being added.
	 */
	template<class System>
	void onComponentAdded(EntityID eid, ComponentID);

	/**
	 * @brief Calls the onComponentRemoved member function on the system.
	 * @tparam System The type of the system.
	 * @param[in] eid The id of the entity being removed from.
	 * @param[in] cid The id of the component being removed.
	 */
	template<class System>
	void onComponentRemoved(EntityID eid, ComponentID cid);

	/**
	 * @brief Calls the onEntityDestroyed member function on the system.
	 * @tparam System The type of the system.
	 * @param[in] eid The id of the entity being destroyed.
	 */
	template<class System>
	void onEntityDestroyed(EntityID eid);

	/**
	 * @brief Calls the run member function on the system.
	 * @tparam System The type of the system.
	 * @param[in] dt The time delta between calls.
	 */
	template<class System>
	void run(float dt);

	/**
	 * @brief Runs onEntityCreated member function on all registered systems.
	 * @param[in] eid The id of the entity being created.
	 */
	void onEntityCreatedAll(EntityID eid);

	/**
	 * @brief Runs onComponentAdded member function on all registered systems.
	 * @param[in] eid The id of the entity being added to.
	 * @param[in] cid The id of the component being added.
	 */
	void onComponentAddedAll(EntityID eid, ComponentID cid);

	/**
	 * @brief Runs onComponentRemoved member function on all registered systems.
	 * @param[in] eid The id of the entity being removed from.
	 * @param[in] cid The id of the component being removed.
	 */
	void onComponentRemovedAll(EntityID eid, ComponentID cid);

	/**
	 * @brief Runs the onEntityDestroyed member function on all registered systems.
	 * @param[in] eid The id of the entity being destroyed.
	 */
	void onEntityDestroyedAll(EntityID eid);

	/**
	 * @brief Runs the run member function on all registered systems.
	 * @param[in] dt The time delta between calls.
	 */
	void runAll(float dt);

	/**
	 * @brief Registers a system for use in the ECS.
	 * @tparam System The system.
	 * @return This is always zero.
	 */
	template<class System, class = std::enable_if_t<IsSystem<System>::value>>
	int registerSystem();
}

#include <Engine/ECS/System.ipp>
