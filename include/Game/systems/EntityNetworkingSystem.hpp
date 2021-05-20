#pragma once

// Game
#include <Game/System.hpp>
#include <Game/comps/ECSNetworkingComponent.hpp>


namespace Game {
	class EntityNetworkingSystem : public System {
		private:
			//ECSNetworkingComponent::Set lastNeighbors;

			// TODO: we should probably have something like this in ecs instead
			// TODO: rm - std::vector<Engine::ECS::ComponentBitset> lastCompsBitsets;

			Engine::Clock::TimePoint nextUpdate = {};

		public:
			using System::System;
			void run(float32 dt);

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
