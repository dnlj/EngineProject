// FMT
#include <fmt/core.h>
#include <fmt/ostream.h>

// Engine
#include <Engine/Gui/DataAdapter.hpp>

// Game
#include <Game/UI/EntityPane.hpp>


namespace {
	namespace EUI = Game::UI::EUI;
	using namespace Engine::Types;

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

			EUI::Panel* createPanel(Id id, EUI::Context& ctx) const {
				auto* base = ctx.constructPanel<EUI::Label>();
				base->autoText(fmt::format("{}", id));
				return base;
			}

	};
}
namespace Game::UI {
	EntityPane::EntityPane(EUI::Context* context) : CollapsibleSection{context} {
		setTitle("Entities");
		auto& world = ctx->getUserdata<EngineInstance>()->getWorld();
		ctx->addPanelUpdateFunc(getContent(), Adapter{world});
		getContent()->setLayout(new EUI::DirectionalLayout{EUI::Direction::Vertical, EUI::Align::Start, EUI::Align::Stretch, ctx->getTheme().sizes.pad1});
	}
}
