#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Gui/Gui.hpp>
#include <Engine/Gui/Panel.hpp>


namespace Engine::Gui {
	struct CellData { // TODO: we also have a struct like this in DirectionalLayout, merge?
		float32 min = 0;
		float32 max = INFINITY;
		float32 val = 0;
		float32 weight = 0;
		float32 pos = 0;
	};

	class GridLayout : public Layout {
		private:
			struct CellMetrics {
				std::vector<CellData> col;
				std::vector<CellData> row;
			};

			CellMetrics cellMetrics;
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
