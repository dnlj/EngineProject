// Engine
#include <Engine/UI/DataAdapter.hpp>

// Game
#include <Game/UI/NetHealthPane.hpp>
#include <Game/systems/NetworkingSystem.hpp>


namespace {
	namespace EUI = Game::UI::EUI;
	using namespace Game;

	class Adapter : public EUI::DataAdapter<Adapter, Engine::Net::IPv4Address, uint64> {
		private:
			Game::World& world;

		public:
			using Cont = std::decay_t<decltype(world.getSystem<NetworkingSystem>().getConnections())>;
			using It = Cont::const_iterator;

			Adapter(Game::World& world) noexcept : world{world} {}

			ENGINE_INLINE auto begin() const { return world.getSystem<NetworkingSystem>().getConnections().begin(); }
			ENGINE_INLINE auto end() const { return world.getSystem<NetworkingSystem>().getConnections().end(); }
			ENGINE_INLINE auto getId(It it) const noexcept { return it->first; }

			Checksum check(Id id) const {
				auto& netSys = world.getSystem<NetworkingSystem>();
				auto* conn = netSys.getConnection(id);

				if (!conn) {
					ENGINE_WARN("Unable to get connection in network health adapter (check). This is a bug.");
					return {};
				}

				uint64 hash = {};
				for (const auto s : conn->getAllChannelQueueSizes()) {
					Engine::hashCombine(hash, s);
				}

				return hash;
			}

			auto createPanel(Id id, It it, EUI::Context* ctx) const {
				auto* base = ctx->constructPanel<EUI::Panel>();
				base->setRelPos({});
				base->setSize({128,128});
				base->setLayout(new EUI::DirectionalLayout{EUI::Direction::Vertical, EUI::Align::Start, EUI::Align::Stretch, ctx->getTheme().sizes.pad1});
				base->setAutoSizeHeight(true);

				auto* ipLabel = ctx->createPanel<EUI::Label>(base);
				ipLabel->autoText(fmt::format("{}", id));
				
				auto& netSys = world.getSystem<NetworkingSystem>();
				auto* conn = netSys.getConnection(id);

				if (!conn) {
					ENGINE_WARN("Unable to get connection in network health adapter (create). This is a bug.");
				} else {
					for (const auto s : conn->getAllChannelQueueSizes()) {
						(void)s;
						ctx->createPanel<EUI::Label>(base);
					}
				}

				updatePanel(id, base);
				return this->group(base);
			}

			void updatePanel(Id id, EUI::Panel* panel) const {
				auto& netSys = world.getSystem<NetworkingSystem>();
				auto* conn = netSys.getConnection(id);

				if (!conn) {
					ENGINE_WARN("Unable to get connection in network health adapter (updatePanel). This is a bug.");
					return;
				}

				EUI::Panel* curr = panel->getFirstChild();
				for (int32 c = 0; const auto s : conn->getAllChannelQueueSizes()) {
					curr = curr->getNextSibling();
					EUI::Label* label = reinterpret_cast<EUI::Label*>(curr);
					label->autoText(fmt::format("Channel {}: {}", c++, s));
				}
			}
	};
}

namespace Game::UI {
	NetHealthPane::NetHealthPane(EUI::Context* context) : CollapsibleSection{context} {
		setTitle("Network Health");
		auto& world = ctx->getUserdata<EngineInstance>()->getWorld();
		ctx->addPanelUpdateFunc(getContent(), Adapter{world});
		getContent()->setLayout(new EUI::DirectionalLayout{EUI::Direction::Vertical, EUI::Align::Start, EUI::Align::Stretch, ctx->getTheme().sizes.pad1});
	}
}
