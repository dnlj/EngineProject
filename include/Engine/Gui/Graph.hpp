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
		protected:
			std::vector<glm::vec2> data;

		public:
			// TODO: this assumes inserting is already sorted
			ENGINE_INLINE void addPoint(glm::vec2 p) { data.push_back(p); };
	};

	class AreaGraph : public SubGraph {
		public:
			void draw(const Panel* panel) const {
				if (data.empty()) { return; }
				auto ctx = panel->getContext();

				const glm::vec2 scale = {1.0f, 1.0f}; // TODO: impl
				const glm::vec2 min = {29, 0};
				const float32 maxX = std::ceil(panel->getWidth() / scale.x);

				const auto h = panel->getHeight();

				const auto end = data.cend();
				auto curr = data.cbegin();
				auto prev = curr;

				while (curr != end && curr->x <= min.x) { ++curr; }
				if (curr == end) { return; }
				if (curr == prev) { ++curr; }
				prev = curr - 1;

				int i = 0;
				while (curr != end) {
					if (prev->x > maxX) { break; }

					glm::vec2 points[] = {
						*prev - min,
						*curr - min,
						glm::vec2{curr->x - min.x, 0},
						glm::vec2{prev->x - min.x, 0},
					};

					for (auto& p : points) {
						p.y = h - p.y;
					}

					ctx->drawPoly(points, {1,0,1,1});

					prev = curr;
					++curr;
				}
			};
	};

	class Graph : public Panel {
		private:
			AreaGraph test;

		public:
			Graph(Context* context) : Panel{context} {
				test.addPoint({0,  10});
				test.addPoint({30, 10});
				test.addPoint({60, 15});
				test.addPoint({90, 25});
			}

			// TODO: do we want an getPoint function like imgui or do we want our own addPoint function?
			// The benefit of doing an addPoint function would be we know exactly when and what data
			// is updated vs polling and doing a full rebuild every frame.
			// The downside is that we have to store duplicate data.
			//
			// I think i want to to the addPoint way. how to handle culling data?

			virtual void render() const override {
				ctx->drawRect({0,0}, getSize(), {0,1,0,0.2});
				test.draw(this);
				//ctx->drawRect
			}

		private:
	};
}
