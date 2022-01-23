#pragma once


namespace Engine::Gui {
	struct FillLayout : Engine::Gui::Layout {
		Engine::float32 pad = 0;

		FillLayout(Engine::float32 padding) : pad{padding} {}
		virtual void layout(Engine::Gui::Panel* panel) override {
			auto* child = panel->getFirstChild();
			if (!child) { return; }
			auto bounds = panel->getBounds();
			bounds.min += pad;
			bounds.max -= pad;
			child->setBounds(bounds);
		}
	};
}
