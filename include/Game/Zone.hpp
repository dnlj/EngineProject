#pragma once

// Game
#include <Game/common.hpp>


namespace Game {
	using ZoneId = uint32;
	using ZoneUnit = int64;
	using ZoneVec = glm::vec<2, ZoneUnit>;
	constexpr inline auto zoneUnitMax = std::numeric_limits<ZoneUnit>::max();
	constexpr inline auto zoneInvalidId = std::numeric_limits<ZoneId>::max();

	class Zone {
		private:
			// TODO: It would be nice if we had a way to just do a intersection by
			//       component on the ECS level instead of storing two sets here.
			Engine::FlatHashSet<Engine::ECS::Entity> players;
			Engine::FlatHashSet<Engine::ECS::Entity> entities;

		public:
			// The offset from the true origin to use for entities in this zone. The effective "position" of this zone.
			ZoneVec offset = {0, 0};

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
}
