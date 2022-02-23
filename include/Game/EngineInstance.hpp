#pragma once

// Engine
#include <Engine/TextureManager.hpp>
#include <Engine/ShaderManager.hpp>
#include <Engine/Camera.hpp>
#include <Engine/Input/BindManager.hpp>


namespace Game {
	enum class InputLayer {
		GuiFocus,
		GuiHover,
		Game,
		_count,
	};

	class EngineInstance {
		public:
			Engine::Input::BindManager bindManager;
			Engine::TextureManager textureManager;
			Engine::ShaderManager shaderManager;
			Engine::Camera camera;
	};
}
