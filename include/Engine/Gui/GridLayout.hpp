#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Gui/common.hpp>
#include <Engine/Gui/Panel.hpp>
#include <Engine/Gui/LayoutMetrics.hpp>


namespace Engine::Gui {
	class GridLayout : public Layout {
		private:
			struct CellMetrics {
				std::vector<LayoutMetrics> col;
				std::vector<LayoutMetrics> row;
			};

			CellMetrics metrics;
			glm::ivec2 dims = {};
			float32 gap = 4;

			// TODO: fixed weighting
			// TODO: cell alignment, start, stretch, end, center, etc

		public:
			// TODO: gap param
			virtual float32 getAutoHeight(const Panel* panel) const { return 53; };
			virtual float32 getAutoWidth(const Panel* panel) const { return 53; };

			virtual void layout(Panel* panel);
	};
}
