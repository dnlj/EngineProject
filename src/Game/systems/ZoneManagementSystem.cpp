// Engine
#include <Engine/Math/math.hpp>

// Game
#include <Game/comps/PhysicsBodyComponent.hpp>
#include <Game/systems/ZoneManagementSystem.hpp>

// GLM
#include <glm/gtx/norm.hpp>

#define ZONE_DEBUG ENGINE_LOG2

namespace {
	using namespace Game;
	class WorldPos {
		public:
			ZoneVec abs;
			glm::vec2 rel;

			ENGINE_INLINE constexpr ZoneVec rough() const noexcept {
				return abs + ZoneVec{rel};
			}
	};

	ENGINE_INLINE constexpr ZoneUnit roughDistance2(const WorldPos& a, const WorldPos& b) noexcept {
		return Engine::Math::length2(a.rough() + b.rough());
	}
}

namespace Game {
	ZoneManagementSystem::ZoneManagementSystem(SystemArg arg)
		: System{arg} {
		// Should always have at least one zone.
		zones.emplace_back();
	}

	void ZoneManagementSystem::tick() {
		// TODO: const auto& playerFilter = world.getFilter<PlayerFlag, PhysicsBodyComponent, ZoneComponent>();
		const auto& playerFilter = world.getFilter<PhysicsBodyComponent, ZoneComponent>();

		//
		// TODO: One option to support higher player counts might be do instead
		// do something like have each zone track its total radius and then use
		// that info as a rough broad phase for which zones potentially need
		// merged or recalculated instead of doing that on a per player basis
		// always.
		// 
		// n = #players
		// m = #zones
		// 
		// Current edge calculations are  n(n-1)/2 = O(n^2)  The proposed method
		// is approximately  n*m  but it is very likely that  m << n  so in
		// reality this is probably closer to something like  n^1.3  which
		// starts paying off around  n=4  and is worst case still only  n^2
		// plus the overhead for zone management. Probably worth looking into if
		// this is a perf issue.
		//
		// Beyond that we could also distribute either the current or proposed
		// method across frames assuming some maximum player move speed. We
		// would need to make sure to account for teleporting between areas
		// though.
		//

		// 
		// TODO (B2R7X87e): One huge downside of this is using squared
		// distances. The second we use the squared distance anywhere in any
		// calculation we limit our maximum world size to radius = sqrt(2^63) =
		// ~3 billion
		//
		// Maybe we can rework this to make sense with Chebyshev or Manhattan
		// distance instead to increase maximum world size? Should probably look
		// into sooner rather than later.
		//
		// I think we would want Chebyshev since it overshoots and Manhattan
		// undershoots? Think about this a bit.
		//

		// TODO: Look into: we really want some kind of spatial data structure
		//       here so we don't have to compute n*m distances. Think physics
		//       quadtree/bsp/broadphase type lookup.

		// TODO: one problem is our world scale is a bit off. Should probably be more like 6-8 blocks per meter instead of 4/5.

		// TODO: cleanup any unused here.
		// All distances are store squared to simplify distance computations.
		constexpr int64 mustSplit2 = Engine::Math::pow2(2000);
		//constexpr int64 likeJoin2 = Engine::Math::pow2(1000);
		constexpr int64 mustJoin2 = Engine::Math::pow2(500);
		static_assert(mustJoin2 < mustSplit2);

		// How close can two zones be to be considered the same. Used to select
		// if an existing zone can be used for a particular group.
		constexpr int64 sameZoneDist2 = Engine::Math::pow2(500);

		// TODo: we can probably combine most of these loops once we have the logic figured out.
		// TODO: look into various clustering algos, k-means, etc.
		// TODO: if this is a perf issue maybe only do one player per tick?
		groupRemaps.clear();
		groupsStorage.clear();
		merged.clear();
		relations.clear();
		ZONE_DEBUG("=========================================");

		// TODO: shouldn't need this. Should be assignd when entity created. Change to a if-constexpr-debug check.
		for (const auto ply : playerFilter) {
			auto& zoneComp = world.getComponent<ZoneComponent>(ply);
			if (zoneComp.zoneId == -1) {
				zoneComp.zoneId = 0;
				zones[0].addPlayer(ply);
			}
		}

		{ // Calculate and cache player to player distances
			auto cur = playerFilter.begin();
			const auto end = playerFilter.end();
			for (; cur != end; ++cur) {
				const auto& physComp1 = world.getComponent<PhysicsBodyComponent>(*cur);
				auto& zoneComp1 = world.getComponent<ZoneComponent>(*cur);
				zoneComp1.group = -1; // Reset for the next pass

				for (auto next = cur; ++next != end;) {
					const auto& physComp2 = world.getComponent<PhysicsBodyComponent>(*next);
					const auto& zoneComp2 = world.getComponent<ZoneComponent>(*next);
					const auto relPos1 = physComp1.getPosition();
					const auto relPos2 = physComp2.getPosition();
					const auto globalBlockPos1 = zones[zoneComp1.zoneId].offset + ZoneVec{relPos1.x, relPos1.y};
					const auto globalBlockPos2 = zones[zoneComp2.zoneId].offset + ZoneVec{relPos2.x, relPos2.y};
					const auto dist2 = Engine::Math::distance2(globalBlockPos1, globalBlockPos2);
					relations.emplace_back(*cur, *next, dist2);
					ZONE_DEBUG("Relation between {} and {} = {} ({})", *cur, *next, dist2, std::sqrt(dist2));
				}
			}

			// TODO: if we merge the must join loop into the above then we should be able to remove this step.
			// Sort to make the distance check logic simpler. Must join, must split, etc. are now all grouped.
			std::ranges::sort(relations, [](const auto& a, const auto& b){
				return a.dist2 < b.dist2;
			});
		}

		{ // Players that need to be in the same zone.
			auto cur = relations.begin();
			const auto end = relations.end();

			// TODO: probably a better scheme for remaps. Need to think a about it a bit.
			// TODO: could just do this step while calcing distances
			for (; cur != end; ++cur) {
				if (cur->dist2 > mustJoin2) { break; }
				auto& zoneComp1 = world.getComponent<ZoneComponent>(cur->ply1);
				auto& zoneComp2 = world.getComponent<ZoneComponent>(cur->ply2);

				// TODO: Test if keeping zone avg totals here is faster than doing it later

				if (zoneComp1.group != -1) { // ply1 is in a group
					if (zoneComp2.group != -1) { // Both are in a group
						if (groupRemaps[zoneComp1.group] == groupRemaps[zoneComp2.group]) { continue; }
						const auto [min, max] = std::minmax(zoneComp1.group, zoneComp2.group);
						ZONE_DEBUG("{} - Merging group {} into {} because of {} and {}", world.getTick(), max, min, cur->ply1, cur->ply2);
						//ENGINE_DEBUG_ASSERT(zoneComp1.group != zoneComp2.group, world.getTick(), " - Attempting to merge group with itself");

						// Merge the larger id group into the smaller id group
						groupsStorage[groupRemaps[min]].append_range(groupsStorage[groupRemaps[max]]);
						groupsStorage[groupRemaps[max]].clear();

						zoneComp1.group = min;
						zoneComp2.group = min;

						// Update any groups that reference max
						for (auto& gid : groupRemaps) {
							if (gid == max) {
								const auto original = std::ranges::find(groupRemaps, gid) - groupRemaps.begin();
								ZONE_DEBUG("    Merge update {} > {} > {}", original, gid, min);
								gid = min;
							}
						}
					} else { // ONLY ply1 is in a group. Use that group.
						zoneComp2.group = zoneComp1.group;
						groupsStorage[groupRemaps[zoneComp1.group]].push_back(cur->ply2);
						ZONE_DEBUG("{} - Adding1 {} to existing group {} with {}", world.getTick(), cur->ply2, zoneComp2.group, cur->ply1);
					}
				} else if (zoneComp2.group != -1) { // Only ply2 is in a group. Use that group.
					zoneComp1.group = zoneComp2.group;
					groupsStorage[groupRemaps[zoneComp2.group]].push_back(cur->ply1);
					ZONE_DEBUG("{} - Adding2 {} to existing group {} with {}", world.getTick(), cur->ply1, zoneComp2.group, cur->ply2);
				} else { // Neither is in a group
					const auto groupId = static_cast<ZoneId>(groupsStorage.size());
					zoneComp1.group = groupId;
					zoneComp2.group = groupId;

					ENGINE_DEBUG_ASSERT(groupsStorage.size() == groupRemaps.size());

					groupRemaps.emplace_back(groupId);
					auto& group = groupsStorage.emplace_back();
					group.push_back(cur->ply1);
					group.push_back(cur->ply2);
					ZONE_DEBUG("{} - Creating new group {} for {} and {}", world.getTick(), zoneComp2.group, cur->ply1, cur->ply2);
				}
			}

		}

		//
		//
		// 
		//
		// TODO: need to handle players where all relations are > mustJoin2 since the above loop drops at mustJoin2.
		// Do we? Wont they just stay where they are which is fine? If so leave a comment explaining
		// We do need to at least clear their group id
		//
		// what if they move outside of 2000m or so? will it split?
		//
		// 
		//
		//
		//
		//

		// Create zones and shift entities.
		for (const auto& groupStorageId : groupRemaps) {
			const auto& group = groupsStorage[groupStorageId];
			if (group.empty()) { continue; }

			// Figure out ideal zone origin.
			ZoneVec ideal = {};
			for (const auto ply : group) {
				const auto& zoneComp = world.getComponent<ZoneComponent>(ply);
				const auto& physComp = world.getComponent<PhysicsBodyComponent>(ply);
				const auto pos = physComp.getPosition();
				ideal += zones[zoneComp.zoneId].offset + ZoneVec{pos.x, pos.y};
			}

			ideal /= group.size();

			// Figure out if an existing zone is close enough to use.
			ZoneId zoneId = -1;
			ZoneUnit minDist2 = sameZoneDist2;
			for (const auto ply : group) {
				const auto& zoneComp = world.getComponent<ZoneComponent>(ply);
				const auto& zone = zones[zoneComp.zoneId];
				const auto dist2 = Engine::Math::distance2(zone.offset, ideal);
				if (dist2 < minDist2) {
					minDist2 = dist2;
					zoneId = zoneComp.zoneId;
				}
			}

			// No existing zone is close enough to use.
			if (zoneId == -1) {
				zoneId = createNewZone(ideal);
				ZONE_DEBUG("{} - Creating new zone: {} @ {}", world.getTick(), zoneId, zones[zoneId].offset);
			} else {
				ZONE_DEBUG("{} - Using existing zone: {} @ {}", world.getTick(), zoneId, zones[zoneId].offset);
			}

			// Migrate or shift
			for (const auto ply : group) {
				auto& zoneComp = world.getComponent<ZoneComponent>(ply);

				// Already in the zone.
				if (zoneComp.zoneId == zoneId) { continue; }

				// Move to new zone
				migratePlayer(ply, zoneId, zoneComp, world.getComponent<PhysicsBodyComponent>(ply));
			}
		}

		for (ZoneId zoneId = 0; zoneId < zones.size(); ++zoneId) {
			if (!zones[zoneId].getPlayers().empty()) { continue; }

			if (zoneId != 0) {
				ZONE_DEBUG("{} - Closing zone {}", world.getTick(), zoneId);

				// TODO: save off entities etc

				zones[zoneId].clear();
				reuse.push_back(zoneId);
			}
		}

		// TODO: migrate/load any non-player entities for zones
	}

	ZoneId ZoneManagementSystem::createNewZone(ZoneVec pos) {
		ZoneId zid = -1;
		if (!reuse.empty()) {
			zid = reuse.back();
			reuse.pop_back();
			return zid;
		} else {
			zid = static_cast<ZoneId>(zones.size());
			zones.emplace_back();
		}

		ENGINE_DEBUG_ASSERT(zid != -1, "Failed to create valid zone. This is a bug.");
		zones[zid].offset = pos;
		return zid;
	}

	void ZoneManagementSystem::migratePlayer(Engine::ECS::Entity ply, ZoneId newZoneId, ZoneComponent& zoneComp, PhysicsBodyComponent& physComp)
	{
		ZONE_DEBUG("{} - Migrating player from {} to {}", world.getTick(), zoneComp.zoneId, newZoneId);
		ENGINE_DEBUG_ASSERT(newZoneId != zoneComp.zoneId, "Attempting to move player to same zone. This is a bug.");

		auto& oldZone = zones[zoneComp.zoneId];
		auto& newZone = zones[newZoneId];

		// TODO: need to do shifting stuff.
		const auto zoneOffsetDiff = newZone.offset - oldZone.offset;
		const b2Vec2 zoneOffsetDiffB2 = {static_cast<float32>(zoneOffsetDiff.x), static_cast<float32>(zoneOffsetDiff.y)};
		physComp.setPosition(physComp.getPosition() - zoneOffsetDiffB2);

		zones[zoneComp.zoneId].removePlayer(ply);
		zones[newZoneId].addPlayer(ply);
		zoneComp.zoneId = newZoneId;

	}
}
