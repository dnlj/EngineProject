#pragma once

// Engine
#include <Engine/StaticVector.hpp>

// Game
#include <Game/System.hpp>
#include <Game/Zone.hpp>


namespace Game {
	class ZoneManagementSystem : public System {
		private:
			using GroupId = uint32;
			using Dist = int64;
			class PlyRel {
				public:
					Engine::ECS::Entity ply1 = {};
					Engine::ECS::Entity ply2 = {};
					WorldAbsUnit dist = std::numeric_limits<WorldAbsUnit>::max();
			};

			class GroupPair {
				public:
					// The src should always be the larger groupId
					GroupId src = -1;
					GroupId dst = -1;
					WorldAbsVec ideal = {};
			};
					
		private:
			std::vector<PlyRel> relations;
			std::vector<ZoneId> reuse;
			std::vector<Zone> zones;

			// Remaps for groups. These map from a groupId to groupStorageId. This extra
			// layer of indirection is needed to handle the case where groups are
			// merged. Its easier to simply update the mapping instead of updating a
			// groupId on everything that could possible be referencing that groupId.
			std::vector<GroupId> groupRemaps;
			std::vector<std::vector<Engine::ECS::Entity>> groupsStorage;

			// Pairs for groups that should be merged.
			std::vector<GroupPair> merged;

		public:
			ZoneManagementSystem(SystemArg arg);
			void setup();
			void tick();
			ENGINE_INLINE const Zone& getZone(ZoneId id) const noexcept { return zones[id]; }
			ENGINE_INLINE void addPlayer(Engine::ECS::Entity ply, ZoneId zoneId) { zones[zoneId].addPlayer(ply); }

			void ensureZone(ZoneId zoneId, WorldAbsVec pos);
			void migratePlayer(Engine::ECS::Entity ply, ZoneId newZoneId, PhysicsBodyComponent& physComp);

		private:
			ZoneId createNewZone(WorldAbsVec pos);
			ENGINE_INLINE void tick_Server();
			ENGINE_INLINE void tick_Client();
	};
}
