#pragma once

// Engine
#include <Engine/StaticVector.hpp>

// Game
#include <Game/System.hpp>
#include <Game/Zone.hpp>


namespace Game {
	class ZoneManagementSystem final : public System {
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
			std::vector<std::vector<Engine::ECS::Entity>> groupStorage;
			std::vector<RealmId> groupRealms;

			// Pairs for groups that should be merged.
			std::vector<GroupPair> merged;

		public:
			ZoneManagementSystem(SystemArg arg);

			void setup();
			void tick();

			ENGINE_INLINE const Zone& getZone(ZoneId id) const noexcept { return zones[id]; }
			ENGINE_INLINE_REL void addPlayer(Engine::ECS::Entity ply, ZoneId zoneId) {
				ENGINE_DEBUG_ASSERT(zoneId < zones.size(), "Attempting to add player to invalid zone.");
				ENGINE_DEBUG_ASSERT(zones[zoneId].active, "Attempting to add player to inactive zone.");
				zones[zoneId].addPlayer(ply);
			}

			/**
			 * Find an existing zone or create a new zone suitable for containing the given position.
			 */
			ENGINE_SERVER_ONLY(ZoneId findOrCreateZoneFor(const RealmId realmId, const WorldAbsVec pos));

			/**
			 * Ensures that a _specific_ given ZoneId is active and offset at the given position.
			 */
			ENGINE_CLIENT_ONLY(void ensureZoneExists(const RealmId realmId, const ZoneId zoneId, const WorldAbsVec pos));
			
			/**
			 * Migrate to a new zone relative to the previous zone.
			 */
			void migratePlayer(const Engine::ECS::Entity ply, const ZoneId newZoneId, PhysicsBodyComponent& physComp);
			void migrateEntity(const Engine::ECS::Entity ent, const ZoneId newZoneId, PhysicsBodyComponent& physComp);

			/**
			 * Migrate to a new zone with a position relative to the new zone offset.
			 */
			void migratePlayer(const Engine::ECS::Entity ply, const ZoneId newZoneId, const WorldVec newPos, PhysicsBodyComponent& physComp);

			/**
			 * Move a player to a given realm at a given position.
			 */
			ENGINE_SERVER_ONLY(void movePlayer(const Engine::ECS::Entity ply, const RealmId realmId, const WorldAbsVec pos));

			const auto& getZones() const { return zones; }

			/**
			 * Increment the external zone reference count.
			 * @see Zone::refCount
			 */
			ENGINE_INLINE_REL void addRef(ZoneId zoneId) {
				ENGINE_DEBUG_ASSERT(zoneId < zones.size());
				++zones[zoneId].refCount;
			}
			
			/**
			 * Decrement the external zone reference count.
			 * @see Zone::refCount
			 */
			ENGINE_INLINE_REL void removeRef(ZoneId zoneId) {
				ENGINE_DEBUG_ASSERT(zoneId < zones.size());
				ENGINE_DEBUG_ASSERT(zones[zoneId].refCount > 0);
				--zones[zoneId].refCount;
			}

		private:
			void migrateEntity(const ZoneId newZoneId, const WorldVec newPos, PhysicsBodyComponent& physComp);

			/**
			 * Provides a central place for activating (or initializing) zones.
			 * This is useful since we have to create and reuse zones from multiple places.
			 */
			ENGINE_INLINE_REL void activateZone(Zone& zone, const RealmId realmId, const WorldAbsVec pos);

			ENGINE_SERVER_ONLY(ENGINE_INLINE_REL ZoneId createNewZone(const RealmId realmId, const WorldAbsVec pos));
			ENGINE_SERVER_ONLY(ENGINE_INLINE_REL void tick_Server());
			ENGINE_CLIENT_ONLY(ENGINE_INLINE_REL void tick_Client());
	};
}
