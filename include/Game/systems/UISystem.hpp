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

namespace Game {
	class UISystem : public System {
		public:
			UISystem(SystemArg arg);
			UISystem(const UISystem&) = delete;
			~UISystem();

			void setup();
			void run(float32 dt);
			void tick();
			void render(RenderLayer layer);

		private:
			Engine::Clock::TimePoint lastUpdate;
			Engine::RingBuffer<std::pair<float32, Engine::Clock::TimePoint>> fpsBuffer;
			float32 fps = 0;

			struct {
				Engine::Gui::Window* window;
				class InfoPane* infoPane;
				class CoordPane* coordPane;
				class CameraPane* cameraPane;
				class NetCondPane* netCondPane;
				class NetHealthPane* netHealthPane;
				class NetGraphPane* netGraphPane;
				class EntityPane* entityPane;

				class MapPreview* mapPreviewWin;

				#if ENGINE_CLIENT
				class ConnectWindow* connectWindow;
				#endif
			} panels;
	};
}
