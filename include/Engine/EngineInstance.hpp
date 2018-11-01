#pragma once

// Engine
#include <Engine/InputManager.hpp>
#include <Engine/TextureManager.hpp>
#include <Engine/ShaderManager.hpp>
#include <Engine/Camera.hpp>

namespace Engine {
	// TODO: Doc
	class EngineInstance {
		public:
			InputManager inputManager;
			TextureManager textureManager;
			ShaderManager shaderManager;
			Camera camera;
	};
}
