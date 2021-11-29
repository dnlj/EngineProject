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
					// TODO: filter out empty polys (x==x or y==y==0)

					ctx->drawPoly(points, color);

					prev = curr;
					++curr;
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
