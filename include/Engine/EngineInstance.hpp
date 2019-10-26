#pragma once

// Engine
#include <Engine/InputManager2.hpp>
#include <Engine/TextureManager.hpp>
#include <Engine/ShaderManager.hpp>
#include <Engine/Camera.hpp>

namespace Engine {
	// TODO: Doc
	class EngineInstance {
		public:
			InputManager2 inputManager;
			TextureManager textureManager;
			ShaderManager shaderManager;
			Camera camera;
	};
}
