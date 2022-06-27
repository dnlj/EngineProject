// Engine
#include <Engine/Gui/GridLayout.hpp>
#include <Engine/ArrayView.hpp>


namespace Engine::Gui {
	void GridLayout::layout(Panel* panel) {
		// Figure out grid dimensions
		{
			glm::ivec2 realDims = {};

			for (auto curr = panel->getFirstChild(); curr; curr = curr->getNextSibling()) {
				realDims = glm::max(realDims, curr->getGridPos());
			}

			realDims.x = dims.x ? dims.x : realDims.x + 1;
			realDims.y = dims.y ? dims.y : realDims.y + 1;

			// Update and clear cell metric storage
			metrics.col.clear();
			metrics.col.resize(realDims.x);
			metrics.row.clear();
			metrics.row.resize(realDims.y);
		}

		// Figure out row/col properties
		auto usableX = panel->getWidth();
		auto usableY = panel->getHeight();

		for (auto curr = panel->getFirstChild(); curr; curr = curr->getNextSibling()) {
			const auto pos = curr->getGridPos();
			auto& col = metrics.col[pos.x];
			auto& row = metrics.row[pos.y];

			// TODO: if we use min or max to clamp the .max value is really matter
			// TODO: cont: of preference and how the child panels should be layed out. Maybe add an option?

			col.min = std::max(col.min, curr->getMinSize().x);
			col.max = std::max(col.max, curr->getMaxSize().x);

			if (const auto w = curr->getWeight(); w == 0) {
				// TODO: use idealSize instead of current size?
				col.val = curr->getWidth();
			} else {
				col.weight = std::max(col.weight, w);
			}

			ENGINE_DEBUG_ASSERT(col.min <= col.max);
				
			row.min = std::max(row.min, curr->getMinSize().y);
			row.max = std::max(row.max, curr->getMaxSize().y);
				
			if (const auto w = curr->getWeight(); w == 0) {
				// TODO: use idealSize instead of current size?
				row.val = curr->getHeight();
			} else {
				row.weight = std::max(row.weight, w);
			}

			ENGINE_DEBUG_ASSERT(row.min <= row.max);
		}

		// Total row/col weights 
		float32 totalColWeight = 0;
		for (auto& col : metrics.col) {
			totalColWeight += col.weight;

			if (col.weight == 0) {
				usableX -= col.val;
			} else {
				col.val = 0;
			}
		}
		
		float32 totalRowWeight = 0;
		for (auto& row : metrics.row) {
			totalRowWeight += row.weight;

			if (row.weight == 0) {
				usableY -= row.val;
			} else {
				row.val = 0;
			}
		}

		// Figure out cell sizes
		LayoutMetrics::distribute(metrics.col, usableX, totalColWeight, gap);
		LayoutMetrics::distribute(metrics.row, usableY, totalRowWeight, gap);

		// Figure out cell positions
		for (int i = 1; i < metrics.col.size(); ++i) {
			metrics.col[i].pos = metrics.col[i-1].pos + metrics.col[i-1].val + gap;
		}

		for (int i = 1; i < metrics.row.size(); ++i) {
			metrics.row[i].pos = metrics.row[i-1].pos + metrics.row[i-1].val + gap;
		}

		// Layout children
		for (auto curr = panel->getFirstChild(); curr; curr = curr->getNextSibling()) {
			const auto pos = curr->getGridPos();

			// TODO: how to size children in cells? stretch? start? stop? i guess we want the same props as DirectionalLayout
			curr->setRelPos({metrics.col[pos.x].pos, metrics.row[pos.y].pos});
			curr->setSize({metrics.col[pos.x].val, metrics.row[pos.y].val});
		}
	}
}
