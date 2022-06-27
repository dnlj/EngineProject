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
#include <Game/World.hpp>
#include <Game/systems/UISystem.hpp>
#include <Game/systems/NetworkingSystem.hpp>
#include <Game/comps/ActionComponent.hpp>
#include <Game/comps/ConnectionComponent.hpp>
#include <Game/comps/NetworkStatsComponent.hpp>
#include <Game/UI/AutoList.hpp>
#include <Game/UI/CoordPane.hpp>
#include <Game/UI/EntityPane.hpp>
#include <Game/UI/NetCondPane.hpp>
#include <Game/UI/NetHealthPane.hpp>
#include <Game/UI/CameraPane.hpp>
#include <Game/UI/ConnectWindow.hpp>


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

	class NetGraphPane : public EUI::CollapsibleSection {
		private:
			class NetGraph : public Panel {
				private:
					EUI::Label* buffer = nullptr;
					EUI::Label* ideal = nullptr;
					EUI::Label* estBuff = nullptr;
					EUI::Label* ping = nullptr;
					EUI::Label* jitter = nullptr;
					EUI::Label* budget = nullptr;
					EUI::Label* sent = nullptr;
					EUI::Label* recv = nullptr;
					EUI::Label* loss = nullptr;
					EUI::Slider* recvRate = nullptr;
					EUI::RichGraph* graph = nullptr;
					EUI::AreaGraph* sentGraphAvg = nullptr;
					EUI::AreaGraph* recvGraphAvg = nullptr;
					EUI::BarGraph* sentGraphDiff = nullptr;
					EUI::BarGraph* recvGraphDiff = nullptr;

					Engine::Clock::TimePoint lastUpdate;

					uint32 lastSentBytes = 0;
					uint32 lastRecvBytes = 0;

				public:
					NetGraph(EUI::Context* context, Engine::ECS::Entity ent, Game::World& world) : Panel{context} {
						setLayout(new EUI::DirectionalLayout{EUI::Direction::Vertical, EUI::Align::Start, EUI::Align::Stretch, 0});
						setAutoSizeHeight(true);

						auto& conn = *world.getComponent<Game::ConnectionComponent>(ent).conn;

						auto* addr = ctx->createPanel<EUI::Label>(this);
						addr->autoText(fmt::format("{}", conn.address()));

						// TODO: if we had full flexbox style layout this would be much simpler. no need for these row containers. This would all work with weights.
						auto* row1 = ctx->createPanel<Panel>(this);
						row1->setLayout(new EUI::DirectionalLayout{EUI::Direction::Horizontal, EUI::Align::Stretch, EUI::Align::Center, 0});
						row1->setAutoSizeHeight(true);
						buffer = ctx->createPanel<EUI::Label>(row1);
						ideal = ctx->createPanel<EUI::Label>(row1);
						estBuff = ctx->createPanel<EUI::Label>(row1);

						auto* row2 = ctx->createPanel<Panel>(this);
						row2->setLayout(new EUI::DirectionalLayout{EUI::Direction::Horizontal, EUI::Align::Stretch, EUI::Align::Center, 0});
						row2->setAutoSizeHeight(true);
						ping = ctx->createPanel<EUI::Label>(row2);
						jitter = ctx->createPanel<EUI::Label>(row2);
						budget = ctx->createPanel<EUI::Label>(row2);

						auto* row3 = ctx->createPanel<Panel>(this);
						row3->setLayout(new EUI::DirectionalLayout{EUI::Direction::Horizontal, EUI::Align::Stretch, EUI::Align::Center, 0});
						row3->setAutoSizeHeight(true);
						sent = ctx->createPanel<EUI::Label>(row3);
						recv = ctx->createPanel<EUI::Label>(row3);
						loss = ctx->createPanel<EUI::Label>(row3);

						auto* row4 = ctx->createPanel<Panel>(this);
						row4->setLayout(new EUI::DirectionalLayout{EUI::Direction::Horizontal, EUI::Align::Stretch, EUI::Align::Center, 0});
						row4->setAutoSizeHeight(true);
						ctx->createPanel<EUI::Label>(row4)->autoText("Packet Recv Rate");
						recvRate = ctx->createPanel<EUI::Slider>(row4);
						recvRate->setWeight(2);
						recvRate->setLimits(1, 255).setValue(0).bind(
							[ent](EUI::Slider& s){
								auto& world = s.getContext()->getUserdata<EngineInstance>()->getWorld();
								if (world.isEnabled(ent) && world.hasComponent<Game::ConnectionComponent>(ent)) {
									auto& conn = *world.getComponent<Game::ConnectionComponent>(ent).conn;
									s.setValue(conn.getPacketRecvRate());
								}
							},
							[ent](EUI::Slider& s){
								auto& world = s.getContext()->getUserdata<EngineInstance>()->getWorld();
								auto& conn = *world.getComponent<Game::ConnectionComponent>(ent).conn;

								if (conn.getState() == ConnectionState::Connected) {
									if (auto msg = conn.beginMessage<MessageType::CONFIG_NETWORK>()) {
										auto v = static_cast<float32>(s.getValue());
										conn.setPacketRecvRate(v);
										msg.write(v);
										return;
									}
								}

								ENGINE_WARN("Unable to set network recv rate!");
							}
						);

						graph = ctx->createPanel<EUI::RichGraph>(this);
						graph->setHeight(200);

						{
							auto graphP = std::make_unique<EUI::AreaGraph>();
							sentGraphAvg = graphP.get();
							sentGraphAvg->max.y = 15000;
							graph->addGraph(std::move(graphP), "Send Avg", true, true);
						}
						{
							auto graphP = std::make_unique<EUI::AreaGraph>();
							recvGraphAvg = graphP.get();
							recvGraphAvg->max.y = 15000;
							graph->addGraph(std::move(graphP), "Recv Avg", false, false);
						}
						{
							auto graphP = std::make_unique<EUI::BarGraph>();
							sentGraphDiff = graphP.get();
							sentGraphDiff->max.y = 500;
							graph->addGraph(std::move(graphP), "Send Bytes", false, true);
						}
						{
							auto graphP = std::make_unique<EUI::BarGraph>();
							recvGraphDiff = graphP.get();
							recvGraphDiff->max.y = 500;
							graph->addGraph(std::move(graphP), "Recv Bytes", false, false);
						}
					}

					void update(Engine::ECS::Entity ent, Game::World& world) {
						// TODO: config time
						const auto now = world.getTime();

						sentGraphAvg->max.x = Engine::Clock::Seconds{now.time_since_epoch()}.count();
						sentGraphAvg->min.x = Engine::Clock::Seconds{(now - std::chrono::milliseconds{10'000}).time_since_epoch()}.count();
						recvGraphAvg->max.x = sentGraphAvg->max.x;
						recvGraphAvg->min.x = sentGraphAvg->min.x;
						sentGraphDiff->max.x = sentGraphAvg->max.x;
						sentGraphDiff->min.x = sentGraphAvg->min.x;
						recvGraphDiff->max.x = sentGraphAvg->max.x;
						recvGraphDiff->min.x = sentGraphAvg->min.x;

						if (!world.hasComponent<NetworkStatsComponent>(ent)) {
							world.addComponent<NetworkStatsComponent>(ent);
						}
						auto& stats = world.getComponent<NetworkStatsComponent>(ent);
						auto& conn = *world.getComponent<Game::ConnectionComponent>(ent).conn;
						
						float32 estbuff = 0;
						if (world.hasComponent<ActionComponent>(ent)) {
							estbuff = world.getComponent<ActionComponent>(ent).estBufferSize;
						}

						const auto sentAvg = conn.getSendBandwidth();
						const auto recvAvg = conn.getRecvBandwidth();

						const auto nowSec = Engine::Clock::Seconds{now.time_since_epoch()}.count();
						sentGraphAvg->addPoint({ nowSec, sentAvg });
						sentGraphAvg->trimData();

						recvGraphAvg->addPoint({ nowSec, recvAvg });
						recvGraphAvg->trimData();

						const auto sentByteDiff = conn.getTotalBytesSent() - lastSentBytes;
						const auto recvByteDiff = conn.getTotalBytesRecv() - lastRecvBytes;

						if (sentByteDiff > 0) {
							sentGraphDiff->addPoint({ nowSec, sentByteDiff });
							lastSentBytes = conn.getTotalBytesSent();
							sentGraphDiff->trimData();
						}

						if (recvByteDiff > 0) {
							recvGraphDiff->addPoint({ nowSec, recvByteDiff });
							lastRecvBytes = conn.getTotalBytesRecv();
							recvGraphDiff->trimData();
						}

						if ((now - lastUpdate) >= std::chrono::milliseconds{100}) {
							lastUpdate = now;
							std::string buff;

							buff.clear(); fmt::format_to(std::back_inserter(buff), "Buffer: {}", stats.inputBufferSize);
							buffer->autoText(buff);
						
							buff.clear(); fmt::format_to(std::back_inserter(buff), "Ideal: {:.3f}", stats.idealInputBufferSize);
							ideal->autoText(buff);

							buff.clear(); fmt::format_to(std::back_inserter(buff), "Est. Buffer: {:.2f}", estbuff);
							estBuff->autoText(buff);

							buff.clear(); fmt::format_to(std::back_inserter(buff), "Ping: {:.1f}ms", Engine::Clock::Seconds{conn.getPing()}.count() * 1000.0f);
							ping->autoText(buff);
						
							buff.clear(); fmt::format_to(std::back_inserter(buff), "Jitter: {:.1f}ms", Engine::Clock::Seconds{conn.getJitter()}.count() * 1000.0f);
							jitter->autoText(buff);

							buff.clear(); fmt::format_to(std::back_inserter(buff), "Budget: {:.2f}", conn.getPacketSendBudget());
							budget->autoText(buff);

							buff.clear(); fmt::format_to(std::back_inserter(buff), "Sent: {}b {:.1f}b/s", conn.getTotalBytesSent(), sentAvg);
							sent->autoText(buff);
						
							buff.clear(); fmt::format_to(std::back_inserter(buff), "Recv: {}b {:.1f}b/s", conn.getTotalBytesRecv(), recvAvg);
							recv->autoText(buff);

							buff.clear(); fmt::format_to(std::back_inserter(buff), "Loss: {:.3f}", conn.getLoss());
							loss->autoText(buff);
						}
					}

			};

			class Adapter : public EUI::DataAdapter<Adapter, Engine::ECS::Entity, int> {
				private:
					Game::World& world;
					int notTheSame = 0;

				public:
					using It = decltype(world.getFilter<ConnectionComponent>().begin());

					Adapter(Game::World& world) noexcept : world{world} {}

					ENGINE_INLINE auto begin() const { return world.getFilter<ConnectionComponent>().begin(); }
					ENGINE_INLINE auto end() const { return world.getFilter<ConnectionComponent>().end(); }
					ENGINE_INLINE auto getId(It it) const noexcept { return *it; }

					Checksum check(Id id) noexcept {
						return ++notTheSame;
					}

					Panel* createPanel(Id id, EUI::Context& ctx) const {
						auto* base = ctx.constructPanel<NetGraph>(id, world);
						updatePanel(id, base);
						return base;
					}

					void updatePanel(Id id, Panel* panel) const {
						reinterpret_cast<NetGraph*>(panel)->update(id, world);
					}

			};
			
		public:
			NetGraphPane(EUI::Context* context) : CollapsibleSection{context} {
				setTitle("Network Graph");
				auto& world = ctx->getUserdata<EngineInstance>()->getWorld();
				ctx->addPanelUpdateFunc(getContent(), Adapter{world});
				getContent()->setLayout(new EUI::DirectionalLayout{EUI::Direction::Vertical, EUI::Align::Start, EUI::Align::Stretch, ctx->getTheme().sizes.pad1});
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
