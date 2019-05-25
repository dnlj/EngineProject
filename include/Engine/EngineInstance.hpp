#pragma once

// Engine
#include <Engine/InputManager.hpp>
#include <Engine/InputManager2.hpp>
#include <Engine/TextureManager.hpp>
#include <Engine/ShaderManager.hpp>
#include <Engine/Camera.hpp>

namespace Engine {
	// TODO: Doc
	class EngineInstance {
		public:
			InputManager inputManager;
			InputManager2 inputManager2;
			TextureManager textureManager;
			ShaderManager shaderManager;
			Camera camera;
	};
}
