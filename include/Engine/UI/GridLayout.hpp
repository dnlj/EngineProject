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
			float32 getAutoDim(const Panel* panel, int dim);
			virtual float32 getAutoWidth(const Panel* panel) override { return getAutoDim(panel, 0); }
			virtual float32 getAutoHeight(const Panel* panel) override { return getAutoDim(panel, 1); }
			virtual void layout(Panel* panel);

		private:
			/**
			 * Resize our CellMetrics for a panel and its siblings.
			 */
			void resizeMetrics(const Panel* panel);
	};
}
