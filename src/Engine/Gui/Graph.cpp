
// Engine
#include <Engine/Gui/Graph.hpp>
#include <Engine/Gui/DirectionalLayout.hpp>
#include <Engine/Gui/GridLayout.hpp>
#include <Engine/Math/Math.hpp>


namespace Engine::Gui {
	void AreaGraph::draw(const Panel* panel) const {
		if (data.empty()) { return; }
		auto ctx = panel->getContext();
		const glm::vec2 scale = panel->getSize() / (max - min);
		const auto h = panel->getHeight();

		const auto end = data.cend();
		auto curr = data.cbegin();
		auto prev = curr;

		while (curr != end && curr->x <= min.x) { ++curr; }
		if (curr == end) { return; }
		if (curr == prev) { ++curr; } else { prev = curr - 1; }

		while (curr != end) {
			if (prev->x > max.x) { break; }

			glm::vec2 points[] = {
				*prev - min,
				*curr - min,
				glm::vec2{curr->x - min.x, 0},
				glm::vec2{prev->x - min.x, 0},
			};

			for (auto& p : points) {
				p.x *= scale.x;
				p.y = h - p.y * scale.y;
			}

			ctx->drawPoly(points, color);

			prev = curr;
			++curr;
		}
	};

	void LineGraph::draw(const Panel* panel) const {
		const auto end = data.cend();
		auto curr = data.cbegin();
		if (end - curr < 2) { return; }

		// TODO: atm we cull first last point to soon. we need to overdraw by one point to get full miters
		while (curr != end && curr->x <= min.x) { ++curr; }
		if (curr == end) { return; }
		if (curr == data.cbegin()) { ++curr; }

		auto ctx = panel->getContext();
		const glm::vec2 scale = panel->getSize() / (max - min);
		const auto h = panel->getHeight();

		// TODO: move into base subgraph class. All graph classes will need something like this.
		// TODO: use lowp_fvec2 for fast norm?
		const auto worldToGraph = [&](auto p) ENGINE_INLINE {
			p = (p - min) * scale;
			p.y = h - p.y;
			return p;
		};

		const auto miterLength = [&](auto aTan, auto bTan) ENGINE_INLINE {
			const auto sinHalfAngleInv = Math::rsqrt(0.5f + 0.5f * glm::dot(aTan, bTan));
			return halfThickness * sinHalfAngleInv;
		};

		const auto nextMiterPoints = [&](auto point, auto pointT, auto lastT) ENGINE_INLINE {
			const auto mT = glm::normalize(pointT + lastT);
			const auto mN = miterLength(pointT, lastT) * glm::vec2{-mT.y, mT.x};
			return std::array{point - mN, point + mN};
		};

		// xV = x's vector, xT = x's tangent, xN = x's normal
		const auto pV = worldToGraph(curr[-1]);
		auto cV = worldToGraph(*curr);
		auto cT = glm::normalize(cV - pV);
		auto pT = cT;
		auto next = curr + 1;
		glm::vec2 nV = {};
		auto [a2, a1] = nextMiterPoints(pV, cT, pT);

		int i = 0; srand(0xDEADBEEF); // TODO: rm
		while (true) {
			cV = worldToGraph(*curr);
			if (next != end) {
				nV = worldToGraph(*next);
				cT = glm::normalize(nV - cV);
			}

			auto [a3, a4] = nextMiterPoints(cV, cT, pT);
			
			//ctx->drawPoly({a1,a2,a3,a4},/*color*/glm::vec4{rand()%256/255.0f,rand()%256/255.0f,rand()%256/255.0f,0.5});
			ctx->drawPoly({a1,a2,a3,a4}, color);
			++i;

			if (next == end) { break; }
			if (curr->x > max.x) { break; }

			a1 = a4;
			a2 = a3;
			pT = cT;
			cV = nV;
			curr = next;
			++next;
		};

		//ENGINE_INFO("Draw Poly x", i);
	};

	void GraphAxis::render() {
		ENGINE_DEBUG_ASSERT(graph);
		const auto& theme = ctx->getTheme();
		ctx->drawRect({}, getSize(), theme.colors.background2);
		const auto min = graph->min[dir];
		const auto max = graph->max[dir];

		const auto range = max - min;
		const auto scale = getSize()[dir] / range;
		const auto line = [&](float32 i, float32 w, float32 l) ENGINE_INLINE {
			const auto p = scale * (dir ? max - i : i - min);
			glm::vec2 a = {};
			a[dir] = p;

			glm::vec2 b = a;
			b[!dir] = l;

			ctx->drawLine(a, b, w, theme.colors.accent);
			return p;
		};

		const auto old = major;
		major = Math::niceNumber(static_cast<float64>(range) / tickGaps);
		const auto nextMajor = std::ceil(min / major) * major;

		if (major != old) {
			for (auto& l : labels) { l.clear(); }
		} else {
			auto diff = nextMajor - labelsStart;
			if (diff < 0) {
				const auto offset = static_cast<int64>(std::round(-diff / major));
				const auto begin = std::shift_right(labels.begin(), labels.end(), offset);
				for (auto it = labels.begin(); it != begin; ++it) {
					it->clear();
				}
			} else if (diff > 0) {
				const auto offset = static_cast<int64>(std::round(diff / major));
				const auto end = std::shift_left(labels.begin(), labels.end(), offset);
				for (auto it = end; it != labels.end(); ++it) {
					it->clear();
				}
			}

		}

		labelsStart = nextMajor;
		const auto count = static_cast<int64>(std::ceil((max - nextMajor) / major));
		for (int64 i = 0; i < count; ++i) {
			auto& label = labels[i];
			const auto v = nextMajor + i * major;

			if (!label.getFont()) {
				label.setFont(ctx->getTheme().fonts.body);
				label = fmt::format("{:.7}", v);
				label.shape();
			}

			auto pos = glm::vec2{0, dir ? 0 : getHeight()};
			pos[dir] = line(static_cast<float32>(v), 2.0f, getSize()[!dir]);
			ctx->drawString(pos, &label);
		}
	}
}

namespace Engine::Gui {
	void GraphArea::GraphAreaImpl::render() {
		const auto& theme = ctx->getTheme();
		ctx->drawRect({0,0}, getSize(), theme.colors.background2);

		for (const auto& graph : graphs) {
			graph->draw(this);
		}
	}
}

namespace Engine::Gui {
	RichGraph::RichGraph(Context* context)
		: Panel{context}
		, area{context->createPanel<GraphArea>(this)} {
		setLayout(new GridLayout{});
	}

	void RichGraph::render() {
		ctx->drawRect({0,0}, getSize(), ctx->getTheme().colors.feature);
		// TODO: current cursor position (ji96pF6X)
	}

	bool RichGraph::onAction(ActionEvent act) {
		if (ctx->getFocus() != this) { return false; }
		// TODO: should we scale scroll by SPI_GETWHEELSCROLLLINES/SPI_GETWHEELSCROLLCHARS?
		switch (act) {
			case Action::Scroll: {
				// Reduce scroll speed
				const auto s = act.value.f32 * 0.5f;

				if (s < 0) {
					scale(1 - s);
				} else {
					scale(1.0f / (1 + s));
				}

				break;
			}
			case Action::ScrollH: {
				break;
			}
			default: { return false; }
		}
		return true;
	}

	// TODO: i think we really want this on right click, maybe this should be an action?
	bool RichGraph::onBeginActivate() {
		if (ctx->getActive() == this) { return true; }

		// TODO: i think we really want this on right/middle click, maybe this should be an action?
		lastDragPos = ctx->getCursor();
		ctx->registerMouseMove(this, [this](const glm::vec2 pos) {
			const auto sz = area->getSize();
			const auto diff = glm::vec2{lastDragPos.x - pos.x, pos.y - lastDragPos.y};
			for (auto& graph : area->getGraphs()) {
				const auto scale = (graph->max - graph->min) / sz;
				const auto offset = diff * scale;
				graph->min += offset;
				graph->max += offset;
			}
			lastDragPos = pos;
		});

		return true;
	}

	void RichGraph::onEndActivate() {
		ctx->deregisterMouseMove(this);
	}

	void RichGraph::scale(float32 s) {
		auto p = ctx->getCursor();
		p -= area->getPos();
		p /= area->getSize();
		p.y = 1.0f - p.y;
		p = glm::clamp(p, 0.0f, 1.0f);

		for (auto& graph : area->getGraphs()) {
			auto size = graph->max - graph->min;
			const auto target = p * size + graph->min; // cursor in world space
			size *= s;
			graph->min = target - size * p;
			graph->max = graph->min + size;
		}
	}

	void RichGraph::addGraph(std::unique_ptr<SubGraph> graph, std::string label) {
		for (auto curr = getFirstChild(); curr; curr = curr->getNextSibling()) {
			curr->setGridColumn(curr->getGridColumn() + 1);
		}

		const int32 count = static_cast<int32>(area->getGraphs().size());

		auto yAxis = ctx->constructPanel<GraphAxis>(Direction::Vertical);
		yAxis->setGraph(graph.get());
		yAxis->setFixedWidth(32);
		yAxis->setWeight(0);
		yAxis->setGridPos(0, 0);

		auto xAxis = ctx->constructPanel<GraphAxis>(Direction::Horizontal);
		xAxis->setGraph(graph.get());
		xAxis->setFixedHeight(16);
		xAxis->setWeight(0);
		xAxis->setGridPos(count+1, count+1);

		addChildren({yAxis, xAxis});
		graph->color = Math::cvtApproxRGBToLinear(Math::cvtHSLtoRGB(nextColorHSL));
		area->addGraph(std::move(graph), std::move(label));
		
		// TODO: let themes specify an array of colors to use for this sort of thing
		nextColorHSL.x = std::fmodf(nextColorHSL.x + InvPhi<float32> * 360, 360);
	}
}
