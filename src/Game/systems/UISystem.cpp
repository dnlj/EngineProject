// Engine
#include <Engine/Gfx/TextureManager.hpp>
#include <Engine/UI/DemoWindow.hpp>
#include <Engine/UI/FillLayout.hpp>
#include <Engine/UI/ScrollArea.hpp>
#include <Engine/UI/TextBox.hpp>
#include <Engine/UI/ConsolePanel.hpp>
#include <Engine/CommandManager.hpp>

// Game
#include <Game/systems/UISystem.hpp>
#include <Game/UI/AutoList.hpp>
#include <Game/UI/CameraPane.hpp>
#include <Game/UI/ConnectWindow.hpp>
#include <Game/UI/ConsoleSuggestionHandler.hpp>
#include <Game/UI/ConsoleWindow.hpp>
#include <Game/UI/CoordPane.hpp>
#include <Game/UI/EntityPane.hpp>
#include <Game/UI/InfoPane.hpp>
#include <Game/UI/NetCondPane.hpp>
#include <Game/UI/NetGraphPane.hpp>
#include <Game/UI/NetHealthPane.hpp>
#include <Game/UI/ResourcePane.hpp>


namespace {
	namespace EUI = Engine::UI;
}

namespace Game {
	UISystem::UISystem(SystemArg arg) : System{arg} {
		auto& ctx = engine.getUIContext();
		auto* prevLastChild = ctx.getRoot()->getLastChildRaw();
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

		{
			panels.mapPreviewWin = ctx.createPanel<UI::MapPreview>(ctx.getRoot());
			panels.mapPreviewWin->setPos({1200, 20});
			panels.mapPreviewWin->setSize({512, 512});
		}

		#if ENGINE_CLIENT
		{
			panels.connectWindow = ctx.createPanel<UI::ConnectWindow>(ctx.getRoot());
		}
		#endif

		{
			panels.consoleWindow = ctx.createPanel<UI::ConsoleWindow>(ctx.getRoot());
			panels.consoleWindow->get()->setAction([](EUI::ConsolePanel* self, std::string_view text) {
				auto* engine = self->getContext()->getUserdata<Game::EngineInstance>();
				engine->getCommandManager().exec(text);
			});

			panels.consoleSuggestionHandler = std::make_unique<UI::ConsoleSuggestionHandler>();
			panels.consoleWindow->get()->setHandler(panels.consoleSuggestionHandler.get());
		}

		//panels.infoPane->toggle();
		//panels.coordPane->toggle();
		panels.netHealthPane->toggle();
		//panels.netCondPane->toggle();
		panels.netGraphPane->toggle();

		{

			struct Texture1 : EUI::Panel {
				Engine::Gfx::Texture2DRef tex;
				Texture1(EUI::Context* context, EngineInstance& engine) : Panel{context} {
					setFixedHeight(128);
					tex = engine.getTextureLoader().get2D("assets/gui_1.bmp");
				}
				void render() override {
					ctx->drawTexture(tex->tex, {}, getSize());
				}
			};
			struct Texture2 : EUI::Panel {
				Engine::Gfx::Texture2DRef tex;
				Texture2(EUI::Context* context, EngineInstance& engine) : Panel{context} {
					setFixedHeight(128);
					tex = engine.getTextureLoader().get2D("assets/gui_2.bmp");
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

		// Hide all panels
		if (!prevLastChild) { prevLastChild = ctx.getRoot()->getFirstChildRaw(); }
		for (auto* curr = prevLastChild; curr; curr = curr->getNextSibling()) {
			curr->setEnabled(false);
		}

		// Show default panels
		panels.window->setEnabled(true);
		#if ENGINE_CLIENT
			panels.connectWindow->setEnabled(true);
			panels.consoleWindow->setEnabled(true);
		#endif
	}

	UISystem::~UISystem() {
	}

	void UISystem::render(RenderLayer layer) {
		if (layer == RenderLayer::UserInterface) {
			engine.getUIContext().render();
		}
	}
}
