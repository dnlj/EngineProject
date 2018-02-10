#pragma once

// STD
#include <array>

// Engine
#include <Engine/ECS/Common.hpp>

namespace Engine::ECS {
	class SystemManager {
		private:
			/**
			 * @brief Gets the next global id to use for systems.
			 * @return The next global id.
			 */
			static SystemID getNextGlobalSystemID();

			/**
			 * @brief Get the global id associated with a system.
			 * @tparam System The system.
			 * @return The global id of @p System.
			 */
			template<class System>
			static SystemID getGlobalSystemID();

		public:
			/**
			 * @brief Get the id associated with a system.
			 * @tparam System The system.
			 * @return The id associated with @p System.
			 */
			template<class System>
			SystemID getSystemID();

		private:
			/** The next id to use for systems */
			SystemID nextID = 0;

			/** The array used for translating from global to local ids */
			std::array<ComponentID, MAX_SYSTEMS> globalToLocalID;

			/**
			 * @brief Gets the next id to use for systems.
			 * @return The next id.
			 */
			SystemID getNextSystemID();

			/**
			 * @brief Get the id associated with a global system id.
			 * @param[in] gsid The global id.
			 * @return The id associated with @p gsid.
			 */
			SystemID getSystemID(SystemID gsid);
	};
}

#include <Engine/ECS/SystemManager.ipp>
