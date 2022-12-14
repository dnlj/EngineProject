#pragma once

// Engine
#include <Engine/FlatHashMap.hpp>

// Game
#include <Game/System.hpp>
#include <Game/comps/ECSNetworkingComponent.hpp>
#include <Game/Connection.hpp>


namespace Game {
	class EntityNetworkingSystem : public System {
		private:
			//ECSNetworkingComponent::Set lastNeighbors;

			// TODO: we should probably have something like this in ecs instead
			// TODO: rm - std::vector<Engine::ECS::ComponentBitset> lastCompsBitsets;

			Engine::Clock::TimePoint nextUpdate = {};
			
			// TODO: at some point we probably want to shrink this
			/** Translate from remote to local entities */
			Engine::FlatHashMap<Engine::ECS::Entity, Engine::ECS::Entity> entRemoteToLocal;

		public:
			using System::System;
			void setup();

			void update(float32 dt);

			auto& getRemoteToLocalEntityMapping() noexcept { return entRemoteToLocal; }
			const auto& getRemoteToLocalEntityMapping() const noexcept { return entRemoteToLocal; }

		#if ENGINE_DEBUG && ENGINE_CLIENT
			bool _debug_networking = false;

			template<class C>
			void onComponentAdded(class Engine::ECS::Entity&) {
				if constexpr (IsNetworkedFlag<C>) {
					ENGINE_DEBUG_ASSERT(_debug_networking, "Attempting to modify networked flag from client side.");
				} else {
					ENGINE_DEBUG_ASSERT(!_debug_networking, "Attempting to modify non-networked flag from network.");
				}
			}

			template<class C>
			void onComponentRemoved(class Engine::ECS::Entity&) {
				if constexpr (IsNetworkedFlag<C>) {
					ENGINE_DEBUG_ASSERT(_debug_networking, "Attempting to modify networked flag from client side.");
				} else {
					ENGINE_DEBUG_ASSERT(!_debug_networking, "Attempting to modify non-networked flag from network.");
				}
			}
		#endif

		private:
			void updateNeighbors();

			void processAddedNeighbors(const Engine::ECS::Entity ply, Connection& conn, ECSNetworkingComponent& ecsNetComp);
			void processRemovedNeighbors(const Engine::ECS::Entity ply, Connection& conn, ECSNetworkingComponent& ecsNetComp);
			void processCurrentNeighbors(const Engine::ECS::Entity ply, Connection& conn, ECSNetworkingComponent& ecsNetComp);

			template<class C>
			[[nodiscard]]
			bool networkComponent(const Engine::ECS::Entity ent, Connection& conn) const;
	};
}
