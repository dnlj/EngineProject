#if ENGINE_OS_WINDOWS
	#include <WinSock2.h>
	#include <Ws2tcpip.h>
#else
	#error Not yet implemented for this operating system.
#endif


// STD
#include <regex>
#include <algorithm>
#include <cstdio>

// FMT
#include <fmt/core.h>
#include <fmt/ostream.h>

// Engine
#include <Engine/Glue/Box2D.hpp>
#include <Engine/Glue/glm.hpp>
#include <Engine/Gui/Context.hpp>
#include <Engine/Gui/Window.hpp>
#include <Engine/Gui/TextBox.hpp>
#include <Engine/Gui/Slider.hpp>
#include <Engine/Gui/DataAdapter.hpp>
#include <Engine/Gui/Graph.hpp>
#include <Engine/Gui/ScrollArea.hpp>
#include <Engine/Gui/ImageDisplay.hpp>
#include <Engine/Gui/DemoWindow.hpp>
#include <Engine/Gfx/TextureLoader.hpp>

// Game
#include <Game/comps/ActionComponent.hpp>
#include <Game/comps/ConnectionComponent.hpp>
#include <Game/comps/NetworkStatsComponent.hpp>
#include <Game/systems/NetworkingSystem.hpp>
#include <Game/systems/UISystem.hpp>
#include <Game/UI/AutoList.hpp>
#include <Game/UI/CameraPane.hpp>
#include <Game/UI/ConnectWindow.hpp>
#include <Game/UI/CoordPane.hpp>
#include <Game/UI/EntityPane.hpp>
#include <Game/UI/NetCondPane.hpp>
#include <Game/UI/NetGraphPane.hpp>
#include <Game/UI/NetHealthPane.hpp>
#include <Game/World.hpp>


namespace {
	namespace EUI = Engine::Gui;
	const double avgDeltaTime = 1/64.0;
}

namespace Game::UI {
	class InfoPane : public AutoList {
		public:
			enum {
				FPS,
				Tick,
				TickScale,
			};

			EUI::Button* disconnect;

			InfoPane(EUI::Context* context) : AutoList{context} {
				setTitle("Info");
				addLabel("FPS: {:.3f} ({:.6f})");
				addLabel("Tick: {}");
				addLabel("Tick Scale: {:.3f}");

				disconnect = ctx->constructPanel<EUI::Button>();
				disconnect->autoText("Disconnect");
				disconnect->lockSize();
				getContent()->addChild(disconnect);
			}
	};
}

namespace Game {
	UISystem::UISystem(SystemArg arg) : System{arg} {
		auto& ctx = engine.getUIContext();
		EUI::Panel* content = nullptr;

		{
			panels.window = ctx.createPanel<EUI::Window>(ctx.getRoot());
			panels.window->setTitle("Debug");
			panels.window->setRelPos({32, 32});
			panels.window->setSize({450, 900});
			panels.window->getContent()->setLayout(new EUI::FillLayout{0});

			auto area = ctx.createPanel<EUI::ScrollArea>(panels.window->getContent());
			content = area->getContent();
			content->setLayout(new EUI::DirectionalLayout{EUI::Direction::Vertical, EUI::Align::Start, EUI::Align::Stretch, ctx.getTheme().sizes.pad1});

			auto text = ctx.createPanel<EUI::TextBox>(content);
			text->setFont(ctx.getTheme().fonts.header);
			text->autoText(R"(Example text)");
			//char8_t str8[] = u8"_a_\u0078\u030A\u0058\u030A_b_!=_===_0xFF_<=_||_++_/=_<<=_<=>_";
			//std::string str = reinterpret_cast<char*>(str8);
			//text->setText(str);
			text->setRelPos({0, 0});
		}

		{
			panels.infoPane = ctx.createPanel<UI::InfoPane>(content);
			panels.infoPane->disconnect->setAction([&](EUI::Button*){
				for (const auto& ent : world.getFilter<ConnectionComponent>()) {
					const auto& addr = world.getComponent<ConnectionComponent>(ent).conn->address();
					world.getSystem<NetworkingSystem>().requestDisconnect(addr);
				}
			});
		}

		{
			panels.coordPane = ctx.createPanel<UI::CoordPane>(content);
			panels.coordPane->setHeight(300);
		}

		if constexpr (ENGINE_SERVER) {
			panels.cameraPane = ctx.createPanel<UI::CameraPane>(content);
		}

		{
			panels.netCondPane = ctx.createPanel<UI::NetCondPane>(content);
			panels.netCondPane->autoHeight();
		}

		{
			panels.netHealthPane = ctx.createPanel<UI::NetHealthPane>(content);
		}

		{
			panels.netGraphPane = ctx.createPanel<UI::NetGraphPane>(content);
		}

		{
			panels.entityPane = ctx.createPanel<UI::EntityPane>(content);
			panels.entityPane->toggle();
		}

		#if ENGINE_CLIENT
		{
			panels.connectWindow = ctx.createPanel<UI::ConnectWindow>(ctx.getRoot());
		}
		#endif

		//panels.infoPane->toggle();
		panels.coordPane->toggle();
		panels.netHealthPane->toggle();
		//panels.netCondPane->toggle();
		panels.netGraphPane->toggle();

		{

			struct Texture1 : EUI::Panel {
				Engine::Gfx::TextureRef tex;
				Texture1(EUI::Context* context, EngineInstance& engine) : Panel{context} {
					setFixedHeight(128);
					tex = engine.getTextureLoader().get("assets/gui_1.bmp");
				}
				void render() override {
					ctx->drawTexture(tex->tex, {}, getSize());
				}
			};
			struct Texture2 : EUI::Panel {
				Engine::Gfx::TextureRef tex;
				Texture2(EUI::Context* context, EngineInstance& engine) : Panel{context} {
					setFixedHeight(128);
					tex = engine.getTextureLoader().get("assets/gui_2.bmp");
				}
				void render() override {
					ctx->drawTexture(tex->tex, {}, getSize());
				}
			};

			ctx.createPanel<Texture1>(content, engine);
			ctx.createPanel<Texture2>(content, engine);
		}

		if (false) {
			auto demo = ctx.createPanel<EUI::DemoWindow>(ctx.getRoot());
			demo->setPos({520, 400});
			demo->setSize({512, 512});
		}
	}

	UISystem::~UISystem() {
	}

	void UISystem::setup() {
	}

	void UISystem::run(float32 dt) {
		const auto now = world.getTime();
		const auto update = now - lastUpdate >= std::chrono::milliseconds{100};
		fpsBuffer.emplace(dt, now);

		// Cull old data
		while(!fpsBuffer.empty() && fpsBuffer.front().second < (now - std::chrono::milliseconds{1000})) {
			fpsBuffer.pop();
		}

		if (update) {
			lastUpdate = now;
			fps = 0;
			for (auto& [delta, time] : fpsBuffer) { fps += delta; }
			fps = fpsBuffer.size() / fps;
		}

		// TODO: use update function
		if (panels.infoPane->getContent()->isEnabled()) {
			if (update) {
				panels.infoPane->setLabel(UI::InfoPane::FPS, fps, 1.0f/fps);
			}

			panels.infoPane->setLabel(UI::InfoPane::Tick, world.getTick());
			panels.infoPane->setLabel(UI::InfoPane::TickScale, world.tickScale);
		}
	}

	void UISystem::render(RenderLayer layer) {
		if (layer == RenderLayer::UserInterface) {
			engine.getUIContext().render();
		}
	}

	void UISystem::tick() {
	}
}
