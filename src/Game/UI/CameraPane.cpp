// FMT
#include <fmt/core.h>
#include <fmt/ostream.h>

// Engine
#include <Engine/Gui/DataAdapter.hpp>

// Game
#include <Game/UI/CameraPane.hpp>


namespace {
	namespace EUI = Game::UI::EUI;
	using namespace Engine::Types;

	class Adapter : public EUI::DataAdapter<Adapter, Engine::ECS::Entity, uint64> {
		private:
			Game::World& world;

		public:
			using It = decltype(world.getFilter<Game::PlayerFlag>().begin());

			Adapter(Game::World& world) noexcept : world{world} {}
			ENGINE_INLINE auto begin() const { return world.getFilter<Game::PlayerFlag>().begin(); }
			ENGINE_INLINE auto end() const { return world.getFilter<Game::PlayerFlag>().end(); }
			ENGINE_INLINE auto getId(It it) const noexcept { return *it; }
			ENGINE_INLINE Checksum check(Id id) const { return *reinterpret_cast<Checksum*>(&id); }

			EUI::Panel* createPanel(Id id, EUI::Context& ctx) const {
				auto* base = ctx.constructPanel<EUI::Button>();
				base->autoText(fmt::format("{}", id));
				base->setAction([id](EUI::Button* btn){
					auto& world = btn->getContext()->getUserdata<Game::EngineInstance>()->getWorld();
					for (const auto ply2 : world.getFilter<Game::PlayerFlag>()) {
						if (world.hasComponent<Game::CameraTargetFlag>(ply2)) {
							world.removeComponent<Game::CameraTargetFlag>(ply2);
						}
					}
					world.addComponent<Game::CameraTargetFlag>(id);
				});
				return base;
			}

	};
}

namespace Game::UI {
	CameraPane::CameraPane(EUI::Context* context) : CollapsibleSection{context} {
		setTitle("Camera");
		auto& world = ctx->getUserdata<EngineInstance>()->getWorld();
		ctx->addPanelUpdateFunc(getContent(), Adapter{world});
		getContent()->setLayout(new EUI::DirectionalLayout{EUI::Direction::Vertical, EUI::Align::Start, EUI::Align::Stretch, ctx->getTheme().sizes.pad1});
	}
}
