
// Engine
#include <Engine/Gui/Graph.hpp>
#include <Engine/Gui/DirectionalLayout.hpp>
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
			return thickness * sinHalfAngleInv;
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

			ctx->drawPoly({a1,a2,a3,a4},/*color*/glm::vec4{rand()%256/255.0f,rand()%256/255.0f,rand()%256/255.0f,0.5});
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

	void GraphAxis::render() const {
		ctx->drawRect({}, getSize(), {0,1,0,1});
		int64 start = min;
		if (auto rem = start % minor; rem != 0) {
			start += rem;
		}

		const auto scale = getWidth() / (max - min);
		const auto line = [&](auto i, auto w, auto h) ENGINE_INLINE {
			const auto x = scale * (i - min);
			ctx->drawLine({x, 0}, {x, h}, w, {1,0,0,0.75});
		};

		for (int64 i = start; i <= max; i += step) {
			if (i % major == 0) {
				// TODO: text labels
				line(i, 2.0f, getHeight());
			} else if (i % minor == 0) {
				line(i, 1.0f, getHeight() * 0.7f);
			}
		}
	}
}

namespace Engine::Gui {
	RichGraph::RichGraph(Context* context)
		: Panel{context}
		, area{context->createPanel<GraphArea>(this)} {
		setLayout(new DirectionalLayout{Direction::Vertical, Align::Stretch, Align::Stretch});
	}

	void RichGraph::render() const {
		ctx->drawRect({0,0}, getSize(), {0,0,1,0.2});
	}

	void RichGraph::onAction(ActionEvent act) {
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
			default: { break; }
		}
	}

	// TODO: i think we really want this on right click, maybe this should be an action?
	void RichGraph::onBeginActivate() {
		if (ctx->getActive() == this) { return; }

		// TODO: i think we really want this on right/middle click, maybe this should be an action?
		lastDragPos = ctx->getCursor();
		ctx->registerMouseMove(this, [this](const glm::vec2 pos) {
			const auto sz = area->getSize();
			const auto diff = glm::vec2{lastDragPos.x - pos.x, pos.y - lastDragPos.y};
			for (auto& graph : area->graphs) {
				const auto scale = (graph->max - graph->min) / sz;
				const auto offset = diff * scale;
				graph->min += offset;
				graph->max += offset;
			}
			lastDragPos = pos;
		});
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

		for (auto& graph : area->graphs) {
			auto size = graph->max - graph->min;
			const auto target = p * size + graph->min; // cursor in world space
			size *= s;
			graph->min = target - size * p;
			graph->max = graph->min + size;
		}
	}
}
