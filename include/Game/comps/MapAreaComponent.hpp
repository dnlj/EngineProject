#pragma once

// Engine
#include <Engine/FlatHashMap.hpp>

namespace Game {
	class MapAreaComponent {
		public:
			struct ChunkMeta {
				Engine::ECS::Tick tick;
				Engine::ECS::Tick last;
			};

			Engine::FlatHashMap<glm::ivec2, ChunkMeta> updates;
	};
}
