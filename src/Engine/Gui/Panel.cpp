// Engine
#include <Engine/Gui/Panel.hpp>
#include <Engine/Gui/Context.hpp>


namespace Engine::Gui {
	Panel::~Panel() {
		ENGINE_DEBUG_ASSERT(ctx, "Panel created with invalid context (null)");
		if (parent) { parent->removeChild(this); }
		if (firstChild) { ctx->deletePanel(firstChild); }
		if (nextSibling) { ctx->deletePanel(nextSibling); }
		delete layout;
	};

	void Panel::render() const {
		ctx->drawRect({0,0}, size, {1,0,0,0.2});
	}
}
