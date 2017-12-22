#pragma once

// Engine
#include <Engine/ECS/ECS.hpp>

#define ENGINE_REGISTER_SYSTEM(s) \
	namespace {\
		const auto __ENGINE_ECS_SYSTEM_DEF__ ## s ## __ = Engine::ECS::detail::registerSystem<s>();\
	}

namespace Engine::ECS {
	/**
	 * @brief Used to check if a type is valid for use as a system.
	 * @tparam T The type to check.
	 */
	template<class T, class = void>
	class IsSystem : public std::false_type {};

	/** @copydoc isSystem */
	template<class T>
	class IsSystem<
		T,
		std::void_t<
		decltype(
			std::declval<T>().onEntityCreated(std::declval<Engine::Entity>()),
			std::declval<T>().onComponentAdded(std::declval<Engine::Entity>(), std::declval<ComponentID>()),
			std::declval<T>().onComponentRemoved(std::declval<Engine::Entity>(), std::declval<ComponentID>()),
			std::declval<T>().onEntityDestroyed(std::declval<Engine::Entity>()),
			std::declval<T>().run(std::declval<float>())
			)
		>
	> : public std::true_type{};
}

namespace Engine::ECS::detail {
	/**
	 * @brief Stores data about a system.
	 */
	class SystemData {
		public:
			using EntityModifyFunction = void(*)(EntityID);
			using ComponentModifyFunction = void(*)(EntityID, ComponentID);
			using RunFunction = void(*)(float);

			SystemData(
				EntityModifyFunction onEntityCreated,
				ComponentModifyFunction onComponentAdded,
				ComponentModifyFunction onComponentRemoved,
				EntityModifyFunction onEntityDestroyed,
				RunFunction run
			);

			const EntityModifyFunction onEntityCreated;
			const ComponentModifyFunction onComponentAdded;
			const ComponentModifyFunction onComponentRemoved;
			const EntityModifyFunction onEntityDestroyed;
			const RunFunction run;
	};

	/** Store data about the registered systems. */
	extern std::vector<SystemData> systemData;

	/**
	 * @brief Registers a system for use in the ECS.
	 * @tparam System The system.
	 * @return This is always zero.
	 */
	template<class System, class = std::enable_if_t<IsSystem<System>::value>>
	int registerSystem();

	/**
	 * @brief Gets the a reference to the system.
	 * @tparam System The type of the system.
	 * @return A reference to the system.
	 */
	template<class System>
	System& getSystem();

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

	// TODO: Doc
	void onEntityCreatedAll(EntityID eid);

	// TODO: Doc
	void onComponentAddedAll(EntityID eid, ComponentID cid);

	// TODO: Doc
	void onComponentRemovedAll(EntityID eid, ComponentID cid);

	// TODO: Doc
	void onEntityDestroyedAll(EntityID eid);

	// TODO: Doc
	void runAll(float dt);
}

#include <Engine/ECS/System.ipp>
