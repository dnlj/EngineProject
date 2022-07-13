// STD
#include <array>

// Engine
#include <Engine/Gfx/TextureLoader.hpp>
#include <Engine/Gfx/VertexLayoutLoader.hpp>
#include <Engine/UI/DataAdapter.hpp>
#include <Engine/UI/DirectionalLayout.hpp>
#include <Engine/UI/Dropdown.hpp>
#include <Engine/UI/GridLayout.hpp>
#include <Engine/FlatHashMap.hpp>

// Game
#include <Game/UI/ResourcePane.hpp>
#include <Game/EngineInstance.hpp>


namespace {
	namespace EUI = Game::UI::EUI;
	using namespace Engine::Types;

	template<class L>
	class Adapter : public EUI::DataAdapter<Adapter<L>, std::string, uint64, false> {
		private:
			L& loader;
			using Id = std::string;
			using It = decltype(loader.begin());
			int nextRow = 0;

		public:
			Adapter(L& loader) : loader{loader} {}
			auto begin() const { return loader.begin(); }
			auto end() const { return loader.end(); }
			auto getId(auto it) const { return it->first; }

			auto check(const Id& id) const {
				auto hash = Engine::hash(id);
				Engine::hashCombine(hash, countOf(id));
				return hash;
			}

			auto createPanel(const Id& id, const It it, EUI::Context* ctx) {
				auto label = ctx->constructPanel<EUI::Label>();
				label->autoText(id);
				label->setWeight(4);
				label->setGridPos(0, nextRow);

				auto count = ctx->constructPanel<EUI::Label>();
				count->autoText(fmt::format("{}", countOf(id)));
				count->setWeight(1);
				count->setGridPos(1, nextRow);

				++nextRow;
				return this->group(label, count);
			}

			void updatePanel(const Id& id, EUI::Panel* panel) {
				auto count = reinterpret_cast<EUI::Label*>(panel->getNextSiblingRaw());
				count->autoText(fmt::format("{}", countOf(id)));
			}

			void remove(const Id& id, EUI::Panel* panel) noexcept {
				const auto row = panel->getGridRow();
				nextRow = 0;

				for (auto& [i, dat] : this->getCache()) {
					const auto first = dat.first;
					const auto last = dat.last;
					auto r = first->getGridRow();

					if (r >= row) {
						--r;
						for (auto curr = first;; curr = curr->getNextSiblingRaw()) {
							curr->setGridRow(r);
							if (curr == last) { break; }
						}
					}

					nextRow = std::max(nextRow, r);
				}

				++nextRow;
			}

		private:
			auto countOf(const Id& id) const {
				// Subtract one for this reference which dies at this return
				return loader.get(id).count() - 1;
			}

	};

	enum class ResourceType {
		None = 0,
		VertexLayout,
		Buffer,
		Shader,
		Texture,
		Mesh,
		Material,
		MaterialInstance,
	};
	ENGINE_BUILD_DECAY_ENUM(ResourceType);
}

namespace Game::UI {
	ResourcePane::ResourcePane(EUI::Context* context) : CollapsibleSection{context} {
		const auto& theme = ctx->getTheme();
		setTitle("Resources (Managed)");

		auto& engine = *ctx->getUserdata<EngineInstance>();
		auto* content = getContent();
		//content->setAutoSize(true);
		content->setLayout(new EUI::DirectionalLayout{EUI::Direction::Vertical, EUI::Align::Start, EUI::Align::Stretch, theme.sizes.pad1});

		auto dd = ctx->constructPanel<EUI::Dropdown>();
		dd->addOption("Texture", +ResourceType::Texture);
		dd->addOption("Option 2", 0);
		dd->addOption("Option 3", 0);
		dd->addOption("Option 4", 0);

		dd->setOnSelection([&](EUI::DropdownOption* opt){
			// TODO: pop to front if exists
			// TODO: if not exists create new window - replace conetens with a pane?
			ctx->clearPanelUpdateFuncs(this);
			for (auto child = body->getLastChild(); child; child = body->getLastChild()) {
				ctx->deletePanel(child);
			}

			switch(static_cast<ResourceType>(opt->id)) {
				case ResourceType::Texture: {
					ctx->addPanelUpdateFunc(body, Adapter{engine.getTextureLoader()});
					break;
				}
			}

			return true;
		});

		body = ctx->constructPanel<EUI::Panel>();
		body->setAutoSizeHeight(true);
		// TODO: really want grid with auto widths
		body->setLayout(new EUI::GridLayout{});

		content->addChildren({dd, body});
	}

	void ResourcePane::render() {
		//ctx->drawRect({0,0}, getSize(), {0,1,0,1});
	}
}
