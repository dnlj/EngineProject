// Engine
#include <Engine/UI/DataAdapter.hpp>

// Game
#include <Game/UI/NetHealthPane.hpp>
#include <Game/comps/ConnectionComponent.hpp>


namespace {
	namespace EUI = Game::UI::EUI;
	using namespace Engine::Types;

	class Adapter : public EUI::DataAdapter<Adapter, Engine::ECS::Entity, uint64> {
		private:
			Game::World& world;

		public:
			using It = decltype(world.getFilter<Game::ConnectionComponent>().begin());

			Adapter(Game::World& world) noexcept : world{world} {}

			ENGINE_INLINE auto begin() const { return world.getFilter<Game::ConnectionComponent>().begin(); }
			ENGINE_INLINE auto end() const { return world.getFilter<Game::ConnectionComponent>().end(); }
			ENGINE_INLINE auto getId(It it) const noexcept { return *it; }

			Checksum check(Id id) const {
				uint64 hash = {};
				auto& conn = *world.getComponent<Game::ConnectionComponent>(id).conn;
				for (const auto s : conn.getAllChannelQueueSizes()) {
					Engine::hashCombine(hash, s);
				}
				return hash;
			}

			auto createPanel(Id id, It it, EUI::Context* ctx) const {
				auto& conn = *world.getComponent<Game::ConnectionComponent>(id).conn;
				const auto& addr = conn.address();

				auto* base = ctx->constructPanel<EUI::Panel>();
				base->setRelPos({});
				base->setSize({128,128});
				base->setLayout(new EUI::DirectionalLayout{EUI::Direction::Vertical, EUI::Align::Start, EUI::Align::Stretch, ctx->getTheme().sizes.pad1});
				base->setAutoSizeHeight(true);

				auto* ipLabel = ctx->createPanel<EUI::Label>(base);
				ipLabel->autoText(fmt::format("{}", addr));

				for (const auto s : conn.getAllChannelQueueSizes()) {
					(void)s;
					ctx->createPanel<EUI::Label>(base);
				}

				updatePanel(id, base);
				return this->group(base);
			}

			void updatePanel(Id id, EUI::Panel* panel) const {
				EUI::Panel* curr = panel->getFirstChild();
				auto& conn = *world.getComponent<Game::ConnectionComponent>(id).conn;
				for (int32 c = 0; const auto s : conn.getAllChannelQueueSizes()) {
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
