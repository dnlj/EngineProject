#pragma once

// Engine
#include <Engine/FlatHashMap.hpp>

namespace Game {
	class MapAreaComponent {
		public:
			struct ChunkMeta {
				Engine::ECS::Tick tick;
				//Engine::Clock::TimePoint time;
				Engine::ECS::Tick last;
			};
			// TODO: set
			Engine::FlatHashMap<glm::ivec2, ChunkMeta> updates;
	};
}
