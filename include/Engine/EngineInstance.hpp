#pragma once

// Engine
#include <Engine/TextureManager.hpp>
#include <Engine/ShaderManager.hpp>
#include <Engine/Camera.hpp>
#include <Engine/Input/InputManager.hpp>

namespace Engine {
	// TODO: Doc
	class EngineInstance {
		public:
			Input::InputManager inputManager;
			TextureManager textureManager;
			ShaderManager shaderManager;
			Camera camera;
	};
}
