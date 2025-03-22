#pragma once

// Game
#include <Game/universal.hpp>

// Engine
#include <Engine/FlatHashMap.hpp>


namespace Game {
	// TODO: This is poorly named.
	/**
	 * Used to track what chunks need to be sent to the client.
	 */
	class MapAreaComponent {
		public:
			struct ChunkMeta {
				/** The last tick this chunk was updated. */
				Engine::ECS::Tick tick;

				/** The last tick this chunk was networked to this particular client. */
				Engine::ECS::Tick last;
			};

			Engine::FlatHashMap<UniversalChunkCoord, ChunkMeta> updates;

		public:
			// TODO: Should probably wrap uses in an ENGINE_SERVER so the code
			//       is excluded from the client entirely.
			MapAreaComponent() {
				ENGINE_DEBUG_ASSERT(ENGINE_SERVER, "MapAreaComponent should only be used on the server side");
			}
	};
}
