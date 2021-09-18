#pragma once

// Engine
#include <Engine/TextureManager.hpp>
#include <Engine/ShaderManager.hpp>
#include <Engine/Camera.hpp>
#include <Engine/Input/InputManager.hpp>


namespace Game {
	class EngineInstance {
		public:
			Engine::Input::InputManager inputManager;
			Engine::TextureManager textureManager;
			Engine::ShaderManager shaderManager;
			Engine::Camera camera;
	};
}
