#pragma once

// Engine
#include <Engine/UI/Window.hpp>

// Game
#include <Game/UI/ui.hpp>

namespace Game::Terrain {
	class TestGenerator;
}

namespace Game::UI {
	class TerrainPreview : public EUI::Window {
		public:
			TerrainPreview(EUI::Context* context);
			Terrain::TestGenerator& generator() noexcept { return *testGenerator; }

		private:
			Terrain::TestGenerator* testGenerator{nullptr};
	};
}
