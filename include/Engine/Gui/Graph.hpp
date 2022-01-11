#pragma once

// Engine
#include <Engine/Gui/Gui.hpp>
#include <Engine/Gui/Panel.hpp>
#include <Engine/Gui/Context.hpp>
#include <Engine/Gui/FillLayout.hpp>


// TODO: split
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
			void draw(const Panel* panel) const;;
	};

	class LineGraph : public SubGraph {
		private:
			// TODO: store `thickness / 2` since thats what we really use this as
			float32 thickness = 10;

		public:
			void draw(const Panel* panel) const;
	};
	
	class GraphArea : public Panel {
		private:
			class GraphAreaImpl : public Panel {
				public:
					std::vector<std::unique_ptr<SubGraph>> graphs;
					using Panel::Panel;
					virtual void render() override {
						ctx->drawRect({0,0}, getSize(), {0,1,0,0.2});
						for (auto& graph : graphs) {
							graph->draw(this);
						}
					}
			};

		private:
			GraphAreaImpl* impl;

		public:
			GraphArea(Context* context)
				: Panel{context}
				, impl{ctx->createPanel<GraphAreaImpl>(this)} {
				setLayout(new FillLayout{0});
			}

			virtual void render() override {
				ctx->drawRect({0,0}, getSize(), {0,1,0,0.2});
			}

			void addGraph(std::unique_ptr<SubGraph> graph) {
				impl->graphs.push_back(std::move(graph));
			}

			auto& getGraphs() noexcept {
				return impl->graphs; 
			}
	};

	class GraphAxis : public Panel {
		private:
			SubGraph* graph = nullptr;
			constexpr static int64 tickGaps = 10;

			/**
			 * Storage for major axis tick mark labels.
			 * 
			 * We can determine the worst case size by looking at the
			 * implementation of `Math::niceNumber`. The worst case is when the
			 * returned number is smallest relative to the input. This happens in
			 * the first case where the output is only 66% of the input. Or when
			 * viewed in the other direction: the input is 150% of the output.
			 * Therefore we need to be able to store 1.5x our ideal tick size.
			 *
			 * The number of ticks = tickGaps + 1
			 */
			std::array<ShapedString, (tickGaps+1) + (tickGaps+1)/2> labels;
			float64 labelsStart = 0;
			float64 major = 10;
			Direction dir = {};

		public:
			GraphAxis(Context* context, Direction dir)
				: Panel{context}
				, dir{dir} {
			}

			virtual void render() override;
			ENGINE_INLINE void setGraph(SubGraph* graph) noexcept { this->graph = graph; }
	};

	class RichGraph : public Panel {
		private: // TODO: private
			GraphArea* area;
			glm::vec2 lastDragPos = {};

		public:
			RichGraph(Context* context);

			virtual void render() override;

			virtual void onAction(ActionEvent act) override;

			virtual void onBeginActivate() override;

			virtual void onEndActivate() override;

			void scale(float32 s);

			void addGraph(std::unique_ptr<SubGraph> graph) {
				//auto axis = ctx->createPanel<GraphAxis>(this, Direction::Horizontal);
				auto axis = ctx->createPanel<GraphAxis>(this, Direction::Vertical);
				axis->setGraph(graph.get());
				//axis->setFixedHeight(16);
				axis->setFixedWidth(64);
				area->addGraph(std::move(graph));
			}

			virtual bool canFocusChild(Panel* child) const { return false; }

		private:
	};
}
