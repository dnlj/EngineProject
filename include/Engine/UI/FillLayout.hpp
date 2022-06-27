#pragma once

// Engine
#include <Engine/Gui/common.hpp>
#include <Engine/Gui/Layout.hpp>
#include <Engine/Gui/Panel.hpp>


namespace Engine::UI {
	struct FillLayout : Engine::UI::Layout {
		Engine::float32 pad = 0;

		FillLayout(Engine::float32 padding) : pad{padding} {}
		virtual void layout(Engine::UI::Panel* panel) override {
			auto* child = panel->getFirstChild();
			if (!child) { return; }
			auto bounds = panel->getBounds();
			bounds.min += pad;
			bounds.max -= pad;
			child->setBounds(bounds);
		}
	};

	template<Direction D>
	struct StretchLayout : Engine::UI::Layout {
		Engine::float32 pad = 0;

		StretchLayout(Engine::float32 padding) : pad{padding} {}
		virtual void layout(Engine::UI::Panel* panel) override {
			auto* child = panel->getFirstChild();
			if (!child) { return; }
			auto sz = child->getSize();
			sz[D] = panel->getSize()[D];
			child->setSize(sz);
		}
	};
}
