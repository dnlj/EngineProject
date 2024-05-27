#pragma once

// Game
#include <Game/System.hpp>
#include <Game/UI/ConnectWindow.hpp>

namespace Engine::UI {
	class Context;
	class Window;
	class RichGraph;
}

namespace Game::UI {
	class InfoPane;
	class CoordPane;
	class CameraPane;
	class NetCondPane;
	class NetHealthPane;
	class NetGraphPane;
	class EntityPane;
	class ConsoleWindow;
	class ConsoleSuggestionHandler;
	class TerrainPreview;
}

namespace Game {
	class UISystem : public System {
		public:
			UISystem(SystemArg arg);
			UISystem(const UISystem&) = delete;
			~UISystem();

			void render(RenderLayer layer);

			UI::ConsoleWindow* getConsole() const noexcept { return panels.consoleWindow; }
			Engine::UI::Window* getDebugWindow() const noexcept { return panels.window; }
			UI::TerrainPreview* getTerrainPreview() const noexcept { return panels.terrainPreviewWin; }

			#if ENGINE_CLIENT
				Engine::UI::Window* getConnectWindow() const noexcept { return panels.connectWindow; }
			#endif

		private:
			struct {
				Engine::UI::Window* window;
				UI::InfoPane* infoPane;
				UI::CoordPane* coordPane;
				UI::CameraPane* cameraPane;
				UI::NetCondPane* netCondPane;
				UI::NetHealthPane* netHealthPane;
				UI::NetGraphPane* netGraphPane;
				UI::EntityPane* entityPane;

				UI::TerrainPreview* terrainPreviewWin;

				#if ENGINE_CLIENT
					UI::ConnectWindow* connectWindow;
				#endif

				std::unique_ptr<UI::ConsoleSuggestionHandler> consoleSuggestionHandler;
				UI::ConsoleWindow* consoleWindow;
			} panels;
	};
}
