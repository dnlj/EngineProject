// Engine
#include <Engine/Gui/Panel.hpp>
#include <Engine/Gui/Context.hpp>


namespace Engine::Gui {
	Panel::~Panel() {
		ENGINE_DEBUG_ASSERT(ctx, "Panel created with invalid context (null)");
		//ENGINE_DEBUG_ASSERT(!firstChild, "Attempting to delete a panel with children");
		//ENGINE_DEBUG_ASSERT(!nextSibling || !ctx->isValid(nextSibling), "Attempting to delete a panel with siblings");
		delete layout;
	};

	void Panel::render() {
		ctx->drawRect({0,0}, size, ctx->getTheme().colors.background);
	}
}
