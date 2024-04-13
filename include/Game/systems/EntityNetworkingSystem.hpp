#pragma once

// Engine
#include <Engine/FlatHashMap.hpp>

// Game
#include <Game/System.hpp>
#include <Game/comps/ECSNetworkingComponent.hpp>
#include <Game/Connection.hpp>


namespace Game {
	class EntityNetworkingSystem : public System {
		public:
			using System::System;

		#if ENGINE_CLIENT
		private:
			// TODO: at some point we probably want to shrink this
			/** Translate from remote to local entities */
			Engine::FlatHashMap<Engine::ECS::Entity, Engine::ECS::Entity> entRemoteToLocal;

		public:
			void setup();

			ENGINE_INLINE void addEntityMapping(Engine::ECS::Entity remote, Engine::ECS::Entity local) {
				[[maybe_unused]] const auto [it, succ] = entRemoteToLocal.try_emplace(remote, local);
				ENGINE_DEBUG_ASSERT(succ, "Attempting to add duplicate mapping for remote entity ", remote, ". Existing = ", it->second, "; New = ", local);
			}

			ENGINE_INLINE Engine::ECS::Entity removeEntityMapping(Engine::ECS::Entity remote) {
				const auto found = entRemoteToLocal.find(remote);
				if (found != entRemoteToLocal.end()) {
					const auto local = found->second;
					entRemoteToLocal.erase(remote);
					return local;
				} else {
					return {};
				}
			}

			ENGINE_INLINE Engine::ECS::Entity getEntityMapping(Engine::ECS::Entity remote) const {
				const auto found = entRemoteToLocal.find(remote);
				return found != entRemoteToLocal.cend() ? found->second : Engine::ECS::Entity{};
			}

			ENGINE_INLINE const auto& getEntityMapping() noexcept {
				return entRemoteToLocal;
			}

			ENGINE_INLINE void clearEntityMapping() {
				entRemoteToLocal.clear();
			}

			#if ENGINE_DEBUG
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
			#endif // ENGINE_DEBUG
		#endif // ENGINE_CLIENT

		#if ENGINE_SERVER
		private:
			Engine::Clock::TimePoint nextUpdate = {};
			std::vector<Engine::ECS::Entity> zoneChanged = {};
			
		public:
			void update(float32 dt);
			void network(NetPlySet plys);

		private:
			void updateNeighbors();

			void processAddedNeighbor(Connection& conn, const Engine::ECS::Entity ent, ECSNetworkingComponent::NeighborData& data);
			void processRemovedNeighbor(Connection& conn, const Engine::ECS::Entity ent, ECSNetworkingComponent::NeighborData& data);
			void processCurrentNeighbor(Connection& conn, const Engine::ECS::Entity ent, ECSNetworkingComponent::NeighborData& data);

			template<class C>
			[[nodiscard]]
			bool networkComponent(const Engine::ECS::Entity ent, Connection& conn) const;
		#endif // ENGINE_SERVER
	};
}
