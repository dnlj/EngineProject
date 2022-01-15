// Engine
#include <Engine/Gui/GridLayout.hpp>
#include <Engine/ArrayView.hpp>

namespace {
	using namespace Engine::Types;
	
	// TODO: we have almost identical logic in DirectionalLayout. move into function or some kind of WeightedLayout base class
	void distributeWeight(
		Engine::ArrayView<Engine::Gui::CellData> data,
		const float32 totalSize,
		const float32 totalWeight,
		const float32 gap) {

		const auto sz = data.size();
		auto remSize = totalSize - gap * (sz - 1);
		auto remWeight = totalWeight;

		while (remSize >= 1 && remWeight > 0) {
			auto runWeight = remWeight;
			for (auto& d : data) {
				const auto w = d.weight / runWeight;
				const auto v = w * remSize;
				const auto o = d.val;
				d.val = std::clamp(d.val + v, d.min, d.max);

				remSize -= d.val - o;
				runWeight -= d.weight;
			}
		}
	}
}

namespace Engine::Gui {
	void GridLayout::layout(Panel* panel) {

		// Figure out grid dimensions
		{
			glm::ivec2 realDims = {};

			auto curr = panel->getFirstChild();
			while (curr) {
				realDims = glm::max(realDims, curr->getGridPos());
				curr = curr->getNextSibling();
			}

			realDims.x = dims.x ? dims.x : realDims.x + 1;
			realDims.y = dims.y ? dims.y : realDims.y + 1;

			// Update and clear cell metric storage
			cellMetrics.col.clear();
			cellMetrics.col.resize(realDims.x);
			cellMetrics.row.clear();
			cellMetrics.row.resize(realDims.y);
		}

		// Figure out row/col properties
		{
			auto curr = panel->getFirstChild();
			while (curr) {
				const auto pos = curr->getGridPos();
				auto& col = cellMetrics.col[pos.x];
				auto& row = cellMetrics.row[pos.y];

				// TODO: if we use min or max to clamp the .max value is really matter
				// TODO: cont: of preference and how the child panels should be layed out. Maybe add an option?

				col.min = std::max(col.min, curr->getMinSize().x);
				col.max = std::max(col.max, curr->getMaxSize().x);

				// TODO: also need to account for gap
				// TODO: also need to account for this weight in total*Weight
				
				// TODO: use idealSize instead of current size if weight 0?
				const auto cw = curr->getWeight() ? curr->getWeight() : curr->getWidth() / panel->getWidth();

				col.weight = std::max(col.weight, cw);
				ENGINE_DEBUG_ASSERT(col.min <= col.max);
				
				row.min = std::max(row.min, curr->getMinSize().y);
				row.max = std::max(row.max, curr->getMaxSize().y);

				// TODO: use idealSize instead of current size if weight 0?
				const auto rw = curr->getWeight() ? curr->getWeight() : curr->getHeight() / panel->getHeight();
				row.weight = std::max(row.weight, rw);
				ENGINE_DEBUG_ASSERT(row.min <= row.max);

				curr = curr->getNextSibling();
			}
		}

		// Total row/col weights 
		float32 totalColWeight = 0;
		for (const auto& col : cellMetrics.col) {
			totalColWeight += col.weight;
		}
		
		float32 totalRowWeight = 0;
		for (const auto& row : cellMetrics.row) {
			totalRowWeight += row.weight;
		}

		// Figure out cell sizes
		distributeWeight(cellMetrics.col, panel->getWidth(), totalColWeight, gap);
		distributeWeight(cellMetrics.row, panel->getHeight(), totalRowWeight, gap);

		// Figure out cell positions
		for (int i = 1; i < cellMetrics.col.size(); ++i) {
			cellMetrics.col[i].pos = cellMetrics.col[i-1].pos + cellMetrics.col[i-1].val + gap;
		}

		for (int i = 1; i < cellMetrics.row.size(); ++i) {
			cellMetrics.row[i].pos = cellMetrics.row[i-1].pos + cellMetrics.row[i-1].val + gap;
		}


		// Layout children
		{
			auto curr = panel->getFirstChild();
			while (curr) {
				const auto pos = curr->getGridPos();
				// TODO: how to size children in cells? stretch? start? stop? i guess we want the same props as DirectionalLayout
				curr->setRelPos({cellMetrics.col[pos.x].pos, cellMetrics.row[pos.y].pos});
				curr->setSize({cellMetrics.col[pos.x].val, cellMetrics.row[pos.y].val});
				curr = curr->getNextSibling();
			}
		}
	}
}
