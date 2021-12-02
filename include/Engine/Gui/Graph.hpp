#pragma once

// Engine
#include <Engine/Gui/Panel.hpp>
#include <Engine/Gfx/Mesh.hpp>


namespace Engine::Gui {
	class GraphVertex {
		public:
			glm::vec2 pos;
			glm::vec4 color;
	};

	class SubGraph {
		public:
			glm::vec2 min = {};
			glm::vec2 max = {100, 100};
			glm::vec4 color = {1,0,0,1};

		protected:
			Engine::RingBuffer<glm::vec2> data;

		public:
			// TODO: this assumes inserting is already sorted
			ENGINE_INLINE void addPoint(glm::vec2 p) { data.push(p); };
			virtual void draw(const Panel* panel) const = 0;

			void trimData() {
				while (true) {
					auto it = data.cbegin();
					if (it == data.cend()) { return; }

					if (++it != data.cend() && it->x <= min.x) {
						data.pop();
					} else {
						return;
					}
				}
			}
	};

	class AreaGraph : public SubGraph {
		public:
			void draw(const Panel* panel) const {
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
	};

	class LineGraph : public SubGraph {
		private:
			// TODO: store `thickness / 2` since thats what we really use this as
			float32 thickness = 10;

		public:
			void draw(const Panel* panel) const {
				const auto end = data.cend();
				auto curr = data.cbegin();
				if (end - curr < 2) { return; }

				auto prev = curr;

				while (curr != end && curr->x <= min.x) { ++curr; }
				if (curr == end) { return; }

				if (curr > prev) { prev = curr - 1; }
				auto next = curr + 1;

				auto ctx = panel->getContext();
				const glm::vec2 scale = panel->getSize() / (max - min);
				const auto h = panel->getHeight();

				// TODO: use lowp_fvec2 for fast norm?
				const auto worldToGraph = [&](auto p) ENGINE_INLINE {
					p = (p - min) * scale;
					p.y = h - p.y;
					return p;
				};

				// TODO: this is actually (and in loop): wrong need to scale normal vec based on angle between tan vecs.
				glm::vec2 pT = glm::normalize(*next - *curr);
				glm::vec2 cT = {};
				glm::vec2 a1 = worldToGraph(*curr) + glm::vec2{pT.y, pT.x};
				glm::vec2 a2 = worldToGraph(*curr) - glm::vec2{pT.y, pT.x};

				while (true) {
					if (prev->x > max.x) { break; }

					if (next == end) {
						cT = {}; // TODO: ?
					} else {
						cT = glm::normalize(*next - *curr);
					}

					// Miter tan/normal
					const auto cMT = glm::normalize(cT + pT);
					const auto cMN = thickness * glm::vec2{cMT.y, cMT.x};

					auto a3 = worldToGraph(*curr);
					const auto a4 = a3 + cMN;
					a3 = a3 - cMN;

					ctx->drawPoly({a1,a2,a3,a4}, color);

					a1 = a4;
					a2 = a3;
					pT = cT;
					prev = curr;
					curr = next;

					if (next == end) { break; }
					++next;
				}
			};
	};

	class Graph : public Panel {
		public: // TODO: private
			std::vector<std::unique_ptr<SubGraph>> graphs;

		public:
			Graph(Context* context) : Panel{context} {
				// TODO: rm 
				//auto test = std::make_unique<AreaGraph>();
				//test->addPoint({0,  10});
				//test->addPoint({30, 10});
				//test->addPoint({60, 15});
				//test->addPoint({90, 25});
				//graphs.push_back(std::move(test));
			}

			// TODO: do we want an getPoint function like imgui or do we want our own addPoint function?
			// The benefit of doing an addPoint function would be we know exactly when and what data
			// is updated vs polling and doing a full rebuild every frame.
			// The downside is that we have to store duplicate data.
			//
			// I think i want to to the addPoint way. how to handle culling data?

			virtual void render() const override {
				ctx->drawRect({0,0}, getSize(), {0,1,0,0.2});
				for (auto& graph : graphs) {
					graph->draw(this);
				}
			}

		private:
	};
}
