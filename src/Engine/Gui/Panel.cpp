// Engine
#include <Engine/Gui/Panel.hpp>
#include <Engine/Gui/Context.hpp>


namespace Engine::Gui {
	Panel::~Panel() {
		ENGINE_DEBUG_ASSERT(ctx, "Panel created with invalid context (null)");
		ENGINE_DEBUG_ASSERT(!firstChild || !ctx->isValid(firstChild), "Attempting to delete a panel with children");
		//ENGINE_DEBUG_ASSERT(!nextSibling || !ctx->isValid(nextSibling), "Attempting to delete a panel with siblings");
		delete layout;
	};

	void Panel::render() const {
		ctx->drawRect({0,0}, size, {1,0,0,0.2});
	}
}
