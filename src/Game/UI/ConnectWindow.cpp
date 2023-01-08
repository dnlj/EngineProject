#if ENGINE_CLIENT

// Engine
#include <Engine/UI/DataAdapter.hpp>
#include <Engine/UI/Button.hpp>
#include <Engine/UI/FillLayout.hpp>
#include <Engine/UI/ScrollArea.hpp>
#include <Engine/UI/TextBox.hpp>

// Game
#include <Game/UI/ConnectWindow.hpp>
#include <Game/systems/NetworkingSystem.hpp>
#include <Game/comps/ConnectionComponent.hpp>

namespace {
	namespace EUI = Game::UI::EUI;
	using namespace Engine::Types;
	using namespace Game;

	bool isAlreadyConnected(Game::World& world, Engine::Net::IPv4Address addr) {
		for (auto ent : world.getFilter<ConnectionComponent>()) {
			auto& connComp = world.getComponent<ConnectionComponent>(ent);
			if (connComp.conn->address() == addr) {
				if (connComp.conn->getState() == ConnectionState::Connected) {
					return true;
				}
			}
		}
		return false;
	}

	auto makeConnectButtonAction(const EUI::StringLine* ipSrc) {
		return [ipSrc](EUI::Button* b){
			auto ctx = b->getContext();
			auto engine = ctx->getUserdata<EngineInstance>();
			auto& world = engine->getWorld();
			auto addr = Engine::Net::hostToAddress(ipSrc->getText());
			if (isAlreadyConnected(world, addr)) { return; }

			// TODO: really would like to be able to clear a single function, doesnt really matter since this is our only callback, but still.
			ctx->addPanelUpdateFunc(b, [last=Engine::Clock::TimePoint{}, count=0u, addr](EUI::Panel* p) mutable {
				auto ctx = p->getContext();
				auto engine = ctx->getUserdata<EngineInstance>();
				auto& world = engine->getWorld();
				auto now = engine->getWorld().getTime();

				if (++count > 5) {
					ENGINE_INFO("Unable to connect to server ", addr); // TODO: might want to have a ui message somewhere (just a top right notification/toast probably)
					ctx->clearPanelUpdateFuncs(p);
					return;
				} else if (isAlreadyConnected(world, addr)) {
					ctx->clearPanelUpdateFuncs(p);
					return;
				}

				if (now - last > std::chrono::milliseconds{200}) {
					last = now;
					world.getSystem<NetworkingSystem>().connectTo(addr);
				}
			});
		};
	}

	class Adapter : public EUI::DataAdapter<Adapter, Engine::Net::IPv4Address, Engine::Net::IPv4Address> {
		private:
			World& world;
			Engine::Clock::TimePoint last;

		public:
			using It = decltype(world.getSystem<NetworkingSystem>().servers.begin());

			Adapter(World& world) noexcept : world{world} {}
			ENGINE_INLINE auto begin() const { return world.getSystem<NetworkingSystem>().servers.begin(); }
			ENGINE_INLINE auto end() const { return world.getSystem<NetworkingSystem>().servers.end(); }
			ENGINE_INLINE auto getId(It it) const noexcept { return it->first; }
			ENGINE_INLINE Checksum check(Id id) const { return id; }

			void update() {
				auto& netSys = world.getSystem<NetworkingSystem>();
				if (netSys.playerCount() == 0 && world.getTime() - last >= std::chrono::seconds{5}) {
					world.getSystem<NetworkingSystem>().broadcastDiscover();
					last = world.getTime();
				}
			}

			auto createPanel(Id id, It it, EUI::Context* ctx) const {
				auto& info = world.getSystem<NetworkingSystem>().servers[id];

				auto labels = ctx->constructPanel<EUI::Panel>();
				labels->setAutoSizeHeight(true);
				labels->setLayout(new EUI::DirectionalLayout{EUI::Direction::Horizontal, EUI::Align::Stretch, EUI::Align::Start, ctx->getTheme().sizes.pad1});
				auto name = ctx->createPanel<EUI::Label>(labels);
				name->autoText(info.name);
				name->setWeight(2);
				ctx->createPanel<EUI::Label>(labels)->autoText("3/16");
				ctx->createPanel<EUI::Label>(labels)->autoText("103ms");

				auto btn = ctx->createPanel<EUI::Button>(labels);
				btn->autoText(fmt::format("{}", id));
				btn->setAction(makeConnectButtonAction(btn));

				return this->group(labels);
			}
	};
}

namespace Game::UI {
	ConnectWindow::ConnectWindow(EUI::Context* context) : Window{context} {
		setTitle("Server List");
		//setSize({300,64});
		setWidth(300);
		setHeight(300);
		setRelPos({512,64});

		getContent()->setLayout(new EUI::FillLayout{0});
		content = ctx->createPanel<EUI::ScrollArea>(getContent())->getContent();
		content->setLayout(new EUI::DirectionalLayout{EUI::Direction::Vertical, EUI::Align::Start, EUI::Align::Stretch, ctx->getTheme().sizes.pad1});

		{
			auto row = ctx->createPanel<EUI::Panel>(content);
			row->setAutoSizeHeight(true);
			row->setLayout(new EUI::DirectionalLayout{EUI::Direction::Horizontal, EUI::Align::Stretch, EUI::Align::Start, ctx->getTheme().sizes.pad1});
					
			auto name = ctx->createPanel<EUI::Label>(row);
			name->autoText("Name");
			name->setWeight(2);
			ctx->createPanel<EUI::Label>(row)->autoText("Players");
			ctx->createPanel<EUI::Label>(row)->autoText("Ping");
			ctx->createPanel<EUI::Label>(row)->autoText("Connect");
		}

		{
			auto row = ctx->createPanel<EUI::Panel>(content);
			row->setAutoSizeHeight(true);

			auto text = ctx->createPanel<EUI::TextBox>(row);
			text->autoText("localhost:21212");

			auto btn = ctx->createPanel<EUI::Button>(row);
			btn->autoText("Connect");
			btn->setFixedWidth(btn->getWidth());
			btn->setAction(makeConnectButtonAction(text));

			row->setLayout(new EUI::DirectionalLayout{EUI::Direction::Horizontal, EUI::Align::Stretch, EUI::Align::Start, ctx->getTheme().sizes.pad1});
		}

		auto& world = ctx->getUserdata<EngineInstance>()->getWorld();
		ctx->addPanelUpdateFunc(content, Adapter{world});
	}
}

#endif
