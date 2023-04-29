// Engine
#include <Engine/UI/Panel.hpp>
#include <Engine/UI/Context.hpp>


namespace Engine::UI {
	Panel::~Panel() {
		ENGINE_DEBUG_ASSERT(ctx, "Panel created with invalid context (null)");
		//ENGINE_DEBUG_ASSERT(!firstChild, "Attempting to delete a panel with children");
		//ENGINE_DEBUG_ASSERT(!nextSibling || !ctx->isValid(nextSibling), "Attempting to delete a panel with siblings");
		delete layout;
	};

	void Panel::render() {
		ctx->setColor(ctx->getTheme().colors.background);
		ctx->drawRect({0,0}, size);
	}

	void PanelC::render() {
		ctx->setColor(color);
		ctx->drawRect({}, getSize());
	}
}
