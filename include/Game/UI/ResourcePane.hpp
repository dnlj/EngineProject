#pragma once

// Engine
#include <Engine/UI/CollapsibleSection.hpp>

// Game
#include <Game/UI/ui.hpp>

namespace Engine::UI {
	class Window;
}

namespace Game::UI {
	enum class ResourceType : uint32 {
		None = 0,
		VertexLayout,
		Buffer,
		Shader,
		Texture,
		Mesh,
		Material,
		MaterialInstance,
		_count,
	};
	ENGINE_BUILD_DECAY_ENUM(ResourceType);

	class ResourcePane : public EUI::CollapsibleSection {
		private:
			EUI::Window* windows[+ResourceType::_count - 1] = {};

		public:
			ResourcePane(EUI::Context* context);
			virtual void render() override;
	};

}
