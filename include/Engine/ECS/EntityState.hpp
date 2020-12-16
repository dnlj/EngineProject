#pragma once

// Engine
#include <Engine/ECS/Common.hpp>


namespace Engine::ECS {
	class EntityState {
		public:
			enum State : uint8 {
				Dead    = 0 << 0,
				Alive   = 1 << 0,
				Enabled = 1 << 1,
			};

			EntityState(Entity ent, State state) : ent{ent}, state{state} {}
			Entity ent; // TODO: could just store generation instead of whole id to save space int state snapshots
			uint8 state;
	};
}
