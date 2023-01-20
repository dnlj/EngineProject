// Engine
#include <Engine/UI/DataAdapter.hpp>
#include <Engine/UI/Graph.hpp>
#include <Engine/UI/Slider.hpp>

// Game
#include <Game/comps/ActionComponent.hpp>
#include <Game/comps/NetworkStatsComponent.hpp>
#include <Game/systems/NetworkingSystem.hpp>
#include <Game/EngineInstance.hpp>
#include <Game/UI/NetGraphPane.hpp>


namespace {
	namespace EUI = Game::UI::EUI;
	using namespace Game;

	class NetGraph : public EUI::Panel {
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
			NetGraph(EUI::Context* context, Engine::Net::IPv4Address addr, Game::World& world) : Panel{context} {
				setLayout(new EUI::DirectionalLayout{EUI::Direction::Vertical, EUI::Align::Start, EUI::Align::Stretch, 0});
				setAutoSizeHeight(true);

				auto& netSys = world.getSystem<NetworkingSystem>();
				auto* conn = netSys.getConnection(addr);
				if (!conn) {
					ENGINE_WARN("Unable to get connection object for entity in NetGraph::NetGraph. This is a bug.");
					return;
				}

				auto* addrLabel = ctx->createPanel<EUI::Label>(this);
				addrLabel->autoText(fmt::format("{}", conn->address()));

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
					[conn](EUI::Slider& s){
						s.setValue(conn->getPacketRecvRate());
					},
					[conn](EUI::Slider& s){
						if (conn->getState() == ConnectionState::Connected) {
							if (auto msg = conn->beginMessage<MessageType::CONFIG_NETWORK>()) {
								auto v = static_cast<float32>(s.getValue());
								conn->setPacketRecvRate(v);
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

			void update(Engine::Net::IPv4Address addr, Game::World& world) {
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
				
				auto& netSys = world.getSystem<NetworkingSystem>();
				auto* conn = netSys.getConnection(addr);

				if (!conn) {
					ENGINE_WARN("Unable to get connection object for entity in NetGraph::update. This is a bug.");
					return;
				}

				const auto* actComp = conn->ent ? world.tryComponent<ActionComponent>(conn->ent) : nullptr;
				const auto* stats = conn->ent ? world.tryComponent<NetworkStatsComponent>(conn->ent) : nullptr;
				if (conn->ent && !stats) {
					stats = &world.addComponent<NetworkStatsComponent>(conn->ent);
				}

				const auto sentAvg = conn->getSendBandwidth();
				const auto recvAvg = conn->getRecvBandwidth();

				const auto nowSec = Engine::Clock::Seconds{now.time_since_epoch()}.count();
				sentGraphAvg->addPoint({ nowSec, sentAvg });
				sentGraphAvg->trimData();

				recvGraphAvg->addPoint({ nowSec, recvAvg });
				recvGraphAvg->trimData();

				const auto sentByteDiff = conn->getTotalBytesSent() - lastSentBytes;
				const auto recvByteDiff = conn->getTotalBytesRecv() - lastRecvBytes;

				if (sentByteDiff > 0) {
					sentGraphDiff->addPoint({ nowSec, sentByteDiff });
					lastSentBytes = conn->getTotalBytesSent();
					sentGraphDiff->trimData();
				}

				if (recvByteDiff > 0) {
					recvGraphDiff->addPoint({ nowSec, recvByteDiff });
					lastRecvBytes = conn->getTotalBytesRecv();
					recvGraphDiff->trimData();
				}

				if ((now - lastUpdate) >= std::chrono::milliseconds{100}) {
					lastUpdate = now;
					std::string buff;

					buff.clear(); fmt::format_to(std::back_inserter(buff), "Buffer: {}", stats ? stats->inputBufferSize : -1);
					buffer->autoText(buff);
						
					buff.clear(); fmt::format_to(std::back_inserter(buff), "Ideal: {:.3f}", stats ? stats->idealInputBufferSize : -1);
					ideal->autoText(buff);

					buff.clear(); fmt::format_to(std::back_inserter(buff), "Est. Buffer: {:.2f}", actComp ? actComp->estBufferSize : -1);
					estBuff->autoText(buff);

					buff.clear(); fmt::format_to(std::back_inserter(buff), "Ping: {:.1f}ms", Engine::Clock::Seconds{conn->getPing()}.count() * 1000.0f);
					ping->autoText(buff);
						
					buff.clear(); fmt::format_to(std::back_inserter(buff), "Jitter: {:.1f}ms", Engine::Clock::Seconds{conn->getJitter()}.count() * 1000.0f);
					jitter->autoText(buff);

					buff.clear(); fmt::format_to(std::back_inserter(buff), "Budget: {:.2f}", conn->getPacketSendBudget());
					budget->autoText(buff);

					buff.clear(); fmt::format_to(std::back_inserter(buff), "Sent: {}b {:.1f}b/s", conn->getTotalBytesSent(), sentAvg);
					sent->autoText(buff);
						
					buff.clear(); fmt::format_to(std::back_inserter(buff), "Recv: {}b {:.1f}b/s", conn->getTotalBytesRecv(), recvAvg);
					recv->autoText(buff);

					buff.clear(); fmt::format_to(std::back_inserter(buff), "Loss: {:.3f}", conn->getLoss());
					loss->autoText(buff);
				}
			}

	};

	class Adapter : public EUI::DataAdapter<Adapter, Engine::Net::IPv4Address, int> {
		private:
			Game::World& world;
			int notTheSame = 0;

		public:
			using Cont = std::decay_t<decltype(world.getSystem<NetworkingSystem>().getConnections())>;
			using It = Cont::const_iterator;

			Adapter(Game::World& world) noexcept : world{world} {}

			ENGINE_INLINE auto begin() const { return world.getSystem<NetworkingSystem>().getConnections().cbegin(); }
			ENGINE_INLINE auto end() const { return world.getSystem<NetworkingSystem>().getConnections().cend(); }
			ENGINE_INLINE auto getId(It it) const noexcept { return it->first; }

			Checksum check(Id id) noexcept {
				return ++notTheSame;
			}

			auto createPanel(Id id, It it, EUI::Context* ctx) const {
				auto* base = ctx->constructPanel<NetGraph>(id, world);
				updatePanel(id, base);
				return this->group(base);
			}

			void updatePanel(Id id, EUI::Panel* panel) const {
				reinterpret_cast<NetGraph*>(panel)->update(id, world);
			}

	};
			
}

namespace Game::UI {
	NetGraphPane::NetGraphPane(EUI::Context* context) : CollapsibleSection{context} {
		setTitle("Network Graph");
		auto& world = ctx->getUserdata<EngineInstance>()->getWorld();
		ctx->addPanelUpdateFunc(getContent(), Adapter{world});
		getContent()->setLayout(new EUI::DirectionalLayout{EUI::Direction::Vertical, EUI::Align::Start, EUI::Align::Stretch, ctx->getTheme().sizes.pad1});
	}
}
