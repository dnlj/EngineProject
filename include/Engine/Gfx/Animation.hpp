#pragma once

// Engine
#include <Engine/Gfx/AnimSeq.hpp>


namespace Engine::Gfx {
	class Animation {
		public:
			float32 duration;
			std::vector<Engine::Gfx::AnimSeq> channels;
	};
}
