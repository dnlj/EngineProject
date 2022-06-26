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
#include <Game/systems/MapSystem.hpp>
#include <Game/systems/PhysicsOriginShiftSystem.hpp>
#include <Game/comps/ActionComponent.hpp>
#include <Game/comps/PhysicsBodyComponent.hpp>
#include <Game/comps/ConnectionComponent.hpp>
#include <Game/comps/NetworkStatsComponent.hpp>
#include <Game/UI/AutoList.hpp>


namespace {
	namespace EUI = Engine::Gui;
	const double avgDeltaTime = 1/64.0;

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

	class CoordPane : public AutoList {
		public:
			enum {
				MouseOffset,
				MouseWorld,
				MouseBlock,
				MouseBlockWorld,
				MouseChunk,
				MouseRegion,
				Camera,
				MapOffset,
				MapOffsetBlock,
				MapOffsetChunk,
			};

			CoordPane(EUI::Context* context) : AutoList{context} {
				setTitle("Coordinates");

				addLabel("Mouse (offset): {:.3f}");
				addLabel("Mouse (world): {:.3f}");
				addLabel("Mouse (block): {}");
				addLabel("Mouse (block-world): {:.3f}");
				addLabel("Mouse (chunk): {} {}");
				addLabel("Mouse (region): {}");
				addLabel("Camera: {:.3f}");
				addLabel("Map Offset: {}");
				addLabel("Map Offset (block): {}");
				addLabel("Map Offset (chunk): {}");

				ctx->addPanelUpdateFunc(this, [](Panel* panel){
					auto pane = reinterpret_cast<CoordPane*>(panel);
					auto& engine = *panel->getContext()->getUserdata<EngineInstance>();
					auto& world = engine.getWorld();

					const auto& activePlayerFilter = world.getFilter<PlayerFlag>();
					if (activePlayerFilter.empty()) { return; }
					const auto ply = *activePlayerFilter.begin();

					auto& mapSys = world.getSystem<Game::MapSystem>();

					const auto& actComp = world.getComponent<Game::ActionComponent>(ply);
					if (!actComp.valid()) { return; }

					// TODO: reimplement - ImGui::Text("Mouse (screen): (%f, %f)", screenMousePos.x, screenMousePos.y);
					const auto& physComp = world.getComponent<PhysicsBodyComponent>(ply);
					const auto offsetMousePos = actComp.getTarget();
					const auto worldMousePos = offsetMousePos + Engine::Glue::as<glm::vec2>(physComp.getPosition());
					const auto blockMousePos = mapSys.worldToBlock(worldMousePos);
					const auto blockWorldMousePos = mapSys.blockToWorld(blockMousePos);
					const auto chunkMousePos = mapSys.blockToChunk(blockMousePos);
					const auto chunkBlockMousePos = mapSys.chunkToBlock(chunkMousePos);
					const auto regionMousePos = mapSys.chunkToRegion(chunkMousePos);
					const auto camPos = engine.getCamera().getPosition();
					const auto mapOffset = world.getSystem<Game::PhysicsOriginShiftSystem>().getOffset();
					const auto mapBlockOffset = mapSys.getBlockOffset();
					const auto mapChunkOffset = mapSys.blockToChunk(mapBlockOffset);

					pane->setLabel(CoordPane::MouseOffset, offsetMousePos);
					pane->setLabel(CoordPane::MouseWorld, worldMousePos);
					pane->setLabel(CoordPane::MouseBlock, blockMousePos);
					pane->setLabel(CoordPane::MouseBlockWorld, blockWorldMousePos);
					pane->setLabel(CoordPane::MouseChunk, chunkMousePos, chunkBlockMousePos);
					pane->setLabel(CoordPane::MouseRegion, regionMousePos);
					pane->setLabel(CoordPane::Camera, camPos);
					pane->setLabel(CoordPane::MapOffset, mapOffset);
					pane->setLabel(CoordPane::MapOffsetBlock, mapBlockOffset);
					pane->setLabel(CoordPane::MapOffsetChunk, mapChunkOffset);
				});
			}
	};

	class NetCondPane : public EUI::CollapsibleSection {
		public:
			NetCondPane(EUI::Context* context) : CollapsibleSection{context} {
				getContent()->setLayout(new EUI::DirectionalLayout{EUI::Direction::Vertical, EUI::Align::Start, EUI::Align::Stretch, ctx->getTheme().sizes.pad1});
				setTitle("Network Conditions");
				
				#ifndef ENGINE_UDP_NETWORK_SIM
					auto label = ctx->createPanel<EUI::Label>(getContent());
					label->autoText("Network simulation disabled.");
				#else
					addSlider("Half Ping Add").setLimits(0, 500).setValue(0).bind(
						[](EUI::Slider& s){
							auto& world = s.getContext()->getUserdata<EngineInstance>()->getWorld();
							auto& settings = world.getSystem<NetworkingSystem>().getSocket().getSimSettings();
							s.setValue(static_cast<float64>(std::chrono::duration_cast<std::chrono::milliseconds>(settings.halfPingAdd).count()));
						},
						[](EUI::Slider& s){
							auto& world = s.getContext()->getUserdata<EngineInstance>()->getWorld();
							auto& settings = world.getSystem<NetworkingSystem>().getSocket().getSimSettings();
							settings.halfPingAdd = std::chrono::milliseconds{static_cast<int64>(s.getValue())};
						}
					);
					addSlider("Jitter").setLimits(0, 1).setValue(0).bind(
						[](EUI::Slider& s){
							auto& world = s.getContext()->getUserdata<EngineInstance>()->getWorld();
							auto& settings = world.getSystem<NetworkingSystem>().getSocket().getSimSettings();
							s.setValue(settings.jitter);
						},
						[](EUI::Slider& s){
							auto& world = s.getContext()->getUserdata<EngineInstance>()->getWorld();
							auto& settings = world.getSystem<NetworkingSystem>().getSocket().getSimSettings();
							settings.jitter = static_cast<float32>(s.getValue());
						}
					);
					addSlider("Duplicate Chance").setLimits(0, 1).setValue(0).bind(
						[](EUI::Slider& s){
							auto& world = s.getContext()->getUserdata<EngineInstance>()->getWorld();
							auto& settings = world.getSystem<NetworkingSystem>().getSocket().getSimSettings();
							s.setValue(settings.duplicate);
						},
						[](EUI::Slider& s){
							auto& world = s.getContext()->getUserdata<EngineInstance>()->getWorld();
							auto& settings = world.getSystem<NetworkingSystem>().getSocket().getSimSettings();
							settings.duplicate = static_cast<float32>(s.getValue());
						}
					);
					addSlider("Loss").setLimits(0, 1).setValue(0).bind(
						[](EUI::Slider& s){
							auto& world = s.getContext()->getUserdata<EngineInstance>()->getWorld();
							auto& settings = world.getSystem<NetworkingSystem>().getSocket().getSimSettings();
							s.setValue(settings.loss);
						},
						[](EUI::Slider& s){
							auto& world = s.getContext()->getUserdata<EngineInstance>()->getWorld();
							auto& settings = world.getSystem<NetworkingSystem>().getSocket().getSimSettings();
							settings.loss = static_cast<float32>(s.getValue());
						}
					);
				#endif
			}

			EUI::Slider& addSlider(std::string_view txt) {
				auto label = ctx->constructPanel<EUI::Label>();
				label->autoText(txt);
				label->setWeight(1);

				auto slider = ctx->constructPanel<EUI::Slider>();
				slider->setWeight(2);
				
				auto line = ctx->createPanel<Panel>(getContent());
				line->setLayout(new EUI::DirectionalLayout{EUI::Direction::Horizontal, EUI::Align::Stretch, EUI::Align::Center, ctx->getTheme().sizes.pad1});
				line->setAutoSizeHeight(true);
				line->addChildren({label, slider});
				return *slider;
			}
	};

	class NetHealthPane : public EUI::CollapsibleSection {
		private:
			class Adapter : public EUI::DataAdapter<Adapter, Engine::ECS::Entity, uint64> {
				private:
					Game::World& world;

				public:
					using It = decltype(world.getFilter<ConnectionComponent>().begin());

					Adapter(Game::World& world) noexcept : world{world} {}

					ENGINE_INLINE auto begin() const { return world.getFilter<ConnectionComponent>().begin(); }
					ENGINE_INLINE auto end() const { return world.getFilter<ConnectionComponent>().end(); }
					ENGINE_INLINE auto getId(It it) const noexcept { return *it; }

					Checksum check(Id id) const {
						uint64 hash = {};
						auto& conn = *world.getComponent<ConnectionComponent>(id).conn;
						for (const auto s : conn.getAllChannelQueueSizes()) {
							Engine::hashCombine(hash, s);
						}
						return hash;
					}

					Panel* createPanel(Id id, EUI::Context& ctx) const {
						auto& conn = *world.getComponent<ConnectionComponent>(id).conn;
						const auto& addr = conn.address();

						auto* base = ctx.constructPanel<Panel>();
						base->setRelPos({});
						base->setSize({128,128});
						base->setLayout(new EUI::DirectionalLayout{EUI::Direction::Vertical, EUI::Align::Start, EUI::Align::Stretch, ctx.getTheme().sizes.pad1});
						base->setAutoSizeHeight(true);

						auto* ipLabel = ctx.createPanel<EUI::Label>(base);
						ipLabel->autoText(fmt::format("{}", addr));

						for (const auto s : conn.getAllChannelQueueSizes()) {
							ctx.createPanel<EUI::Label>(base);
						}

						updatePanel(id, base);
						return base;
					}

					void updatePanel(Id id, Panel* panel) const {
						Panel* curr = panel->getFirstChild();
						auto& conn = *world.getComponent<ConnectionComponent>(id).conn;
						for (int32 c = 0; const auto s : conn.getAllChannelQueueSizes()) {
							curr = curr->getNextSibling();
							EUI::Label* label = reinterpret_cast<EUI::Label*>(curr);
							label->autoText(fmt::format("Channel {}: {}", c++, s));
						}
					}

			};
			
		public:
			NetHealthPane(EUI::Context* context) : CollapsibleSection{context} {
				setTitle("Network Health");
				auto& world = ctx->getUserdata<EngineInstance>()->getWorld();
				ctx->addPanelUpdateFunc(getContent(), Adapter{world});
				getContent()->setLayout(new EUI::DirectionalLayout{EUI::Direction::Vertical, EUI::Align::Start, EUI::Align::Stretch, ctx->getTheme().sizes.pad1});
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

	class EntityPane : public EUI::CollapsibleSection {
		private:
			class Adapter : public EUI::DataAdapter<Adapter, Engine::ECS::Entity, uint64> {
				private:
					Game::World& world;

				public:
					using It = decltype(world.getEntities().begin());

					Adapter(Game::World& world) noexcept : world{world} {}
					ENGINE_INLINE auto begin() const { return world.getEntities().begin(); }
					ENGINE_INLINE auto end() const { return world.getEntities().end(); }
					ENGINE_INLINE auto getId(It it) const noexcept { return it->ent; }
					ENGINE_INLINE bool filter(Id id) const noexcept { return world.isAlive(id); }
					ENGINE_INLINE Checksum check(Id id) const { return *reinterpret_cast<Checksum*>(&id);	}

					Panel* createPanel(Id id, EUI::Context& ctx) const {
						auto* base = ctx.constructPanel<EUI::Label>();
						base->autoText(fmt::format("{}", id));
						return base;
					}

			};

		public:
			EntityPane(EUI::Context* context) : CollapsibleSection{context} {
				setTitle("Entities");
				auto& world = ctx->getUserdata<EngineInstance>()->getWorld();
				ctx->addPanelUpdateFunc(getContent(), Adapter{world});
				getContent()->setLayout(new EUI::DirectionalLayout{EUI::Direction::Vertical, EUI::Align::Start, EUI::Align::Stretch, ctx->getTheme().sizes.pad1});
			}
	};

	class CameraPane : public EUI::CollapsibleSection {
		private:
			class Adapter : public EUI::DataAdapter<Adapter, Engine::ECS::Entity, uint64> {
				private:
					Game::World& world;

				public:
					using It = decltype(world.getFilter<PlayerFlag>().begin());

					Adapter(Game::World& world) noexcept : world{world} {}
					ENGINE_INLINE auto begin() const { return world.getFilter<PlayerFlag>().begin(); }
					ENGINE_INLINE auto end() const { return world.getFilter<PlayerFlag>().end(); }
					ENGINE_INLINE auto getId(It it) const noexcept { return *it; }
					ENGINE_INLINE Checksum check(Id id) const { return *reinterpret_cast<Checksum*>(&id); }

					Panel* createPanel(Id id, EUI::Context& ctx) const {
						auto* base = ctx.constructPanel<EUI::Button>();
						base->autoText(fmt::format("{}", id));
						base->setAction([id](EUI::Button* btn){
							auto& world = btn->getContext()->getUserdata<EngineInstance>()->getWorld();
							for (const auto ply2 : world.getFilter<PlayerFlag>()) {
								if (world.hasComponent<CameraTargetFlag>(ply2)) {
									world.removeComponent<CameraTargetFlag>(ply2);
								}
							}
							world.addComponent<CameraTargetFlag>(id);
						});
						return base;
					}

			};

		public:
			CameraPane(EUI::Context* context) : CollapsibleSection{context} {
				setTitle("Camera");
				auto& world = ctx->getUserdata<EngineInstance>()->getWorld();
				ctx->addPanelUpdateFunc(getContent(), Adapter{world});
				getContent()->setLayout(new EUI::DirectionalLayout{EUI::Direction::Vertical, EUI::Align::Start, EUI::Align::Stretch, ctx->getTheme().sizes.pad1});
			}
	};

	#if ENGINE_CLIENT
	class ConnectWindow : public EUI::Window {
		private:
			class Adapter : public EUI::DataAdapter<Adapter, Engine::Net::IPv4Address, Engine::Net::IPv4Address> {
				private:
					Game::World& world;
					Engine::Clock::TimePoint last;

				public:
					using It = decltype(world.getSystem<NetworkingSystem>().servers.begin());

					Adapter(Game::World& world) noexcept : world{world} {}
					ENGINE_INLINE auto begin() const { return world.getSystem<NetworkingSystem>().servers.begin(); }
					ENGINE_INLINE auto end() const { return world.getSystem<NetworkingSystem>().servers.end(); }
					ENGINE_INLINE auto getId(It it) const noexcept { return it->first; }
					ENGINE_INLINE Checksum check(Id id) const { return id; }

					void update() {
						auto& netSys = world.getSystem<Game::NetworkingSystem>();
						if (netSys.playerCount() == 0 && world.getTime() - last >= std::chrono::seconds{5}) {
							world.getSystem<Game::NetworkingSystem>().broadcastDiscover();
							last = world.getTime();
						}
					}

					Panel* createPanel(Id id, EUI::Context& ctx) const {
						auto& info = world.getSystem<NetworkingSystem>().servers[id];

						auto labels = ctx.constructPanel<EUI::Panel>();
						labels->setAutoSizeHeight(true);
						labels->setLayout(new EUI::DirectionalLayout{EUI::Direction::Horizontal, EUI::Align::Stretch, EUI::Align::Start, ctx.getTheme().sizes.pad1});
						auto name = ctx.createPanel<EUI::Label>(labels);
						name->autoText(info.name);
						name->setWeight(2);
						ctx.createPanel<EUI::Label>(labels)->autoText("3/16");
						ctx.createPanel<EUI::Label>(labels)->autoText("103ms");

						auto btn = ctx.createPanel<EUI::Button>(labels);
						btn->autoText(fmt::format("{}", id));
						btn->setAction([id](EUI::Button* b){
							auto engine = b->getContext()->getUserdata<EngineInstance>();
							connectTo(b->getText(), *engine);
						});

						return labels;
					}
			};
		private:
			EUI::Panel* content = nullptr;

		public:
			ConnectWindow(EUI::Context* context) : Window{context} {
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
	};
	#endif
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
