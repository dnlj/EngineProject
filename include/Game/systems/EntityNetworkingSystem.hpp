#pragma once

// Game
#include <Game/System.hpp>
#include <Game/comps/NeighborsComponent.hpp>


namespace Game {
	class EntityNetworkingSystem : public System {
		private:
			NeighborsComponent::Set lastNeighbors;

			// TODO: we should probably have something like this in ecs instead
			std::vector<Engine::ECS::ComponentBitset> lastCompsBitsets;

			Engine::Clock::TimePoint nextUpdate = {};

		public:
			using System::System;
			void run(float32 dt);

		private:
			void updateNeighbors();

			void processAddedNeighbors(const Engine::ECS::Entity ply, Connection& conn, NeighborsComponent& neighComp);
			void processRemovedNeighbors(const Engine::ECS::Entity ply, Connection& conn, NeighborsComponent& neighComp);
			void processCurrentNeighbors(const Engine::ECS::Entity ply, Connection& conn, NeighborsComponent& neighComp);
	};
}
