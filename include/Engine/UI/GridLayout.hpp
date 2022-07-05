#pragma once

// Engine
#include <Engine/Engine.hpp>
#include <Engine/UI/common.hpp>
#include <Engine/UI/Panel.hpp>
#include <Engine/UI/LayoutMetrics.hpp>


namespace Engine::UI {
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