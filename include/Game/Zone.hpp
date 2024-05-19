#pragma once

// Game
#include <Game/common.hpp>


namespace Game {
	class Zone {
		private:
			// TODO: It would be nice if we had a way to just do a intersection by
			//       component on the ECS level instead of storing two sets here.
			Engine::FlatHashSet<Engine::ECS::Entity> players;

		public:
			/** The offset from the true origin to use for entities in this zone. The effective "position" of this zone. */
			WorldAbsVec offset = {0, 0};

			/** Is this zone being used? */
			bool active = false;

			/**
			 * External references other than players or entities. Used to
			 * prevent unloading while still referenced by external systems.
			 * Used by active map chunks for example.
			 */
			uint32 refCount = 0;

			RealmId realmId = 0;

			ENGINE_INLINE void addPlayer(Engine::ECS::Entity ply) {
				ENGINE_DEBUG_ASSERT(!players.contains(ply), "Attempting to insert duplicate player in zone. This is a bug.");
				players.insert(ply);
			};

			ENGINE_INLINE void removePlayer(Engine::ECS::Entity ply) {
				ENGINE_DEBUG_ASSERT(players.contains(ply), "Attempting to remove player not in zone. This is a bug.");
				players.erase(ply);
			};

			ENGINE_INLINE const auto& getPlayers() const noexcept { return players; }

			ENGINE_INLINE void clear() noexcept {
				players.clear();
			}
	};
}
