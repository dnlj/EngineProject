#pragma once

// STD
#include <cmath>

// Engine
#include <Engine/RingBuffer.hpp>
#include <Engine/ImGui/ImGui.hpp>

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

			ENGINE_INLINE auto& getContext() noexcept { return *ctx; }
			ENGINE_INLINE auto& getWorld() const noexcept { return world; }
			ENGINE_INLINE auto& getEngine() const noexcept { return engine; }

		private:
			Engine::Gui::Context* ctx;
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

			template<bool B>
			static ImPlotPoint netGetPointAvg(void* data, int idx);

			template<bool B>
			static ImPlotPoint netGetDiff(void* data, int idx);
	};
}
