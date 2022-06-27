#pragma once

// Engine
#include <Engine/RingBuffer.hpp>

// Game
#include <Game/System.hpp>

namespace Engine::Gui {
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
}

namespace Game {
	class UISystem : public System {
		public:
			UISystem(SystemArg arg);
			UISystem(const UISystem&) = delete;
			~UISystem();

			void setup();
			void update(float32 dt);
			void tick();
			void render(RenderLayer layer);

		private:
			Engine::Clock::TimePoint lastUpdate;
			Engine::RingBuffer<std::pair<float32, Engine::Clock::TimePoint>> fpsBuffer;
			float32 fps = 0;

			struct {
				Engine::Gui::Window* window;
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
			} panels;
	};
}
