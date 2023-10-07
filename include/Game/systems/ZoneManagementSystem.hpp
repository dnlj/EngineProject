#pragma once

// Engine
#include <Engine/StaticVector.hpp>

// Game
#include <Game/System.hpp>


namespace Game {
	using ZoneId = uint32;
	using ZoneUnit = int64;
	using ZoneVec = glm::vec<2, ZoneUnit>;
	constexpr inline auto zoneUnitMax = std::numeric_limits<ZoneUnit>::max();

	class Zone {
		private:
			// TODO: It would be nice if we had a way to just do a intersection by
			//       component on the ECS level instead of storing two sets here.
			Engine::FlatHashSet<Engine::ECS::Entity> players;
			Engine::FlatHashSet<Engine::ECS::Entity> entities;

		public:
			// The offset from the true origin to use for entities in this zone. The effective "position" of this zone.
			ZoneVec offset;

			ENGINE_INLINE void addPlayer(Engine::ECS::Entity ply) {
				ENGINE_DEBUG_ASSERT(!players.contains(ply), "Attempting to insert duplicate player in zone. This is a bug.");
				players.insert(ply);
			};

			ENGINE_INLINE void removePlayer(Engine::ECS::Entity ply) {
				ENGINE_DEBUG_ASSERT(players.contains(ply), "Attempting to remove player not in zone. This is a bug.");
				players.erase(ply);
			};

			ENGINE_INLINE const auto& getPlayers() const noexcept { return players; }
			ENGINE_INLINE const auto& getEntities() const noexcept { return entities; }

			ENGINE_INLINE void clear() noexcept {
				players.clear();
				entities.clear();
			}
	};

	class ZoneComponent {
		public:
			ZoneUnit dist2 = zoneUnitMax;
			ZoneId zoneId = -1;

			// TODO: should probably be a temp DS on the manager.
			ZoneId group = -1;
	};

	class ZoneManagementSystem : public System {
		private:
			using GroupId = uint32;
			using Dist = int64;
			class PlyRel {
				public:
					Engine::ECS::Entity ply1 = {};
					Engine::ECS::Entity ply2 = {};
					Dist dist2 = zoneUnitMax;
			};

			class GroupPair {
				public:
					// The src should always be the larger groupId
					GroupId src = -1;
					GroupId dst = -1;
					ZoneVec ideal = {};
			};
					
		private:
			std::vector<PlyRel> relations;
			std::vector<ZoneId> reuse;
			std::vector<Zone> zones;
			//Engine::StaticVector<ZonePID, std::numeric_limits<ZonePID>::digits> groups;

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
			void tick();

		private:
			ZoneId createNewZone(ZoneVec pos);
			void migratePlayer(Engine::ECS::Entity ply, ZoneId newZoneId, ZoneComponent& zoneComp, PhysicsBodyComponent& physComp);
	};
}
