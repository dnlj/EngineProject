#pragma once

// Engine
#include <Engine/Gfx/TextureType.hpp>


namespace Engine::Gfx {
	template<class Tex>
	class TextureInfo {
		public:
			Tex tex;
			glm::ivec3 size;
			TextureType type;
	};
}
