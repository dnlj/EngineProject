#pragma once

// Game
#include <Game/System.hpp>

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
	class MapPreview;
	class ConnectWindow;
	class ConsoleWindow;
}

namespace Game {
	class UISystem : public System {
		public:
			UISystem(SystemArg arg);
			UISystem(const UISystem&) = delete;

			void render(RenderLayer layer);

			UI::ConsoleWindow* getConsole() const noexcept { return panels.consoleWindow; }

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

				UI::MapPreview* mapPreviewWin;

				#if ENGINE_CLIENT
				UI::ConnectWindow* connectWindow;
				#endif

				UI::ConsoleWindow* consoleWindow;
			} panels;
	};
}
