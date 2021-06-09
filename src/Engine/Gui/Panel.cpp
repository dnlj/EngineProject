// Engine
#include <Engine/Gui/Panel.hpp>
#include <Engine/Gui/Context.hpp>


namespace Engine::Gui {
	Panel::~Panel() {
		delete firstChild;
		delete nextSibling;
	};

	void Panel::render(Context& ctx) const {
		ctx.addRect(pos, size);
	}
}
