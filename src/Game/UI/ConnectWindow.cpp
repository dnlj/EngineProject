#if ENGINE_CLIENT

// STD
#include <regex>

// Engine
#include <Engine/UI/DataAdapter.hpp>
#include <Engine/UI/Button.hpp>
#include <Engine/UI/FillLayout.hpp>
#include <Engine/UI/ScrollArea.hpp>
#include <Engine/UI/TextBox.hpp>

// Game
#include <Game/UI/ConnectWindow.hpp>
#include <Game/systems/NetworkingSystem.hpp>

namespace {
	namespace EUI = Game::UI::EUI;
	using namespace Engine::Types;

	bool connectTo(const std::string& uri, Game::EngineInstance& engine) {
		Game::World& world = engine.getWorld();

		// TODO: use Engine::Net::hostToAddress
		addrinfo* results = nullptr;
		addrinfo hints = {
			.ai_family = AF_INET, // TODO: support ipv6 - AF_UNSPEC
			.ai_socktype = SOCK_DGRAM,
		};

		std::regex regex{R"(^(?:(.*):\/\/)?(.*?)(?::(\d+))?(?:\/.*)?$)"};
		std::smatch matches;
		std::regex_match(uri, matches, regex);
		ENGINE_ASSERT_WARN(matches.size() == 4, "Incorrect number of captures");

		std::string host = matches[2].str();
		std::string serv = matches[3].matched ? matches[3].str() : matches[1].str();

		if (auto err = getaddrinfo(host.data(), serv.data(), &hints, &results); err) {
			ENGINE_WARN("Address error");
			// TODO: error log message + popup/notification
			return false;
		} else {
			for (auto ptr = results; ptr; ptr = results->ai_next) {
				if (ptr->ai_family != AF_INET) {
					ENGINE_ERROR("Unsuported network family");
				}

				Engine::Net::IPv4Address addr{*ptr->ai_addr};
				ENGINE_LOG("Address: ", addr);
				world.getSystem<Game::NetworkingSystem>().connectTo(addr);
			}
		}

		freeaddrinfo(results);
		return true;
	}

	class Adapter : public EUI::DataAdapter<Adapter, Engine::Net::IPv4Address, Engine::Net::IPv4Address> {
		private:
			Game::World& world;
			Engine::Clock::TimePoint last;

		public:
			using It = decltype(world.getSystem<Game::NetworkingSystem>().servers.begin());

			Adapter(Game::World& world) noexcept : world{world} {}
			ENGINE_INLINE auto begin() const { return world.getSystem<Game::NetworkingSystem>().servers.begin(); }
			ENGINE_INLINE auto end() const { return world.getSystem<Game::NetworkingSystem>().servers.end(); }
			ENGINE_INLINE auto getId(It it) const noexcept { return it->first; }
			ENGINE_INLINE Checksum check(Id id) const { return id; }

			void update() {
				auto& netSys = world.getSystem<Game::NetworkingSystem>();
				if (netSys.playerCount() == 0 && world.getTime() - last >= std::chrono::seconds{5}) {
					world.getSystem<Game::NetworkingSystem>().broadcastDiscover();
					last = world.getTime();
				}
			}

			auto createPanel(Id id, It it, EUI::Context* ctx) const {
				auto& info = world.getSystem<Game::NetworkingSystem>().servers[id];

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
				btn->setAction([id](EUI::Button* b){
					auto engine = b->getContext()->getUserdata<Game::EngineInstance>();
					connectTo(b->getText(), *engine);
				});

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
			btn->setAction([text](EUI::Button* b){
				auto engine = b->getContext()->getUserdata<EngineInstance>();
				connectTo(text->getText(), *engine);
			});

			row->setLayout(new EUI::DirectionalLayout{EUI::Direction::Horizontal, EUI::Align::Stretch, EUI::Align::Start, ctx->getTheme().sizes.pad1});
		}

		auto& world = ctx->getUserdata<EngineInstance>()->getWorld();
		ctx->addPanelUpdateFunc(content, Adapter{world});
	}
}

#endif
