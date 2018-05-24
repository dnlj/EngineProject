#pragma once

// Engine
#include <Engine/InputManager.hpp>
#include <Engine/TextureManager.hpp>

namespace Engine {
	// TODO: Doc
	class EngineInstance {
		public:
			InputManager inputManager;
			TextureManager textureManager;
	};
}
