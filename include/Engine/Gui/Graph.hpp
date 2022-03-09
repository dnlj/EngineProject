#pragma once

// GLM
#include <glm/gtc/color_space.hpp>

// Engine
#include <Engine/Gui/common.hpp>
#include <Engine/Gui/Panel.hpp>
#include <Engine/Gui/Context.hpp>
#include <Engine/Gui/FillLayout.hpp>
#include <Engine/Gui/Button.hpp>
#include <Engine/Gui/DirectionalLayout.hpp>
#include <Engine/Math/color.hpp>


namespace Engine::Gui {
	class GraphVertex {
		public:
			glm::vec2 pos;
			glm::vec4 color;
	};

	/**
	 * TODO: doc
	 * TODO: note about updating graph min/max
	 * TODO: note abotu trimming data
	 */
	class SubGraph {
		public:
			glm::vec2 min = {};
			glm::vec2 max = {100, 100};

			glm::vec4 color = {1,0,0,1}; // TODO: private
			bool enabled = true; // TODO: private
			glm::vec2 scale = {1,1}; // TODO: private

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

	class BarGraph : public SubGraph {
		public:
			void draw(const Panel* panel) const override;
	};

	class AreaGraph : public SubGraph {
		public:
			void draw(const Panel* panel) const override;
	};

	class LineGraph : public SubGraph {
		private:
			float32 halfThickness = 2;

		public:
			void draw(const Panel* panel) const override;
			void setLineThickness(const float32 thickness) noexcept { halfThickness = thickness * 0.5f; }
	};
	
	class GraphKey : public PanelT {
		private:
			class Swatch : public Panel {
				public:
					using Panel::Panel;
					glm::vec4 color = {1,0,0,1};
					std::function<void(Swatch*)> activeCallback;

					bool onBeginActivate() override {
						if (activeCallback) { activeCallback(this); }
						return true;
					}

					void render() override {
						ctx->drawRect({}, getSize(), color);
					}
			};

		public:
			GraphKey(Context* context) : PanelT{context} {
				setLayout(new DirectionalLayout{Direction::Vertical, Align::Start, Align::Start, ctx->getTheme().sizes.pad1});
				setAutoSize(true);
			}

			void add(SubGraph* graph, std::string text, glm::vec4 color) {
				auto base = ctx->createPanel<Panel>(this);
				base->setLayout(new DirectionalLayout{Direction::Horizontal, Align::Start, Align::Start, ctx->getTheme().sizes.pad1});
				base->setAutoSize(true);

				auto swatch = ctx->createPanel<Swatch>(base);
				auto label = ctx->createPanel<Label>(base);
				label->autoText(std::move(text));
				swatch->setFixedSize({label->getHeight(), label->getHeight()});
				swatch->color = color;
				swatch->activeCallback = [graph](Swatch* s){ graph->enabled = !graph->enabled; };
			}
	};

	class GraphArea : public Panel {
		private:
			class GraphAreaImpl : public Panel {
				public:
					std::vector<std::unique_ptr<SubGraph>> graphs;
					using Panel::Panel;
					virtual void render() override;
					virtual bool canHover() const override { return false; }
					virtual bool canFocus() const override { return false; }
			};

		private:
			GraphAreaImpl* impl = nullptr;
			GraphKey* key = nullptr;

		public:
			GraphArea(Context* context)
				: Panel{context}
				, impl{ctx->createPanel<GraphAreaImpl>(this)} {
				setLayout(new FillLayout{0});

				impl->setLayout(new FillLayout{0});
				key = ctx->createPanel<GraphKey>(impl);
			}

			virtual void render() override {}

			void addGraph(std::unique_ptr<SubGraph> graph, std::string label) {
				key->add(graph.get(), std::move(label), graph->color);
				impl->graphs.push_back(std::move(graph));
			}

			auto& getGraphs() noexcept {
				return impl->graphs; 
			}
			
			virtual bool canHover() const override { return false; }
			virtual bool canFocus() const override { return false; }
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
			 * Therefor we need to be able to store 1.5x our ideal tick size.
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
		private:
			GraphArea* area;
			glm::vec2 lastDragPos = {};
			
			// Ideally we would use HCL instead of HSL due to better maintained
			// perceived brightness and saturation between hues, but from a quick glance
			// the math looks much more involved.
			glm::vec4 nextColorHSL = {3, 0.65, 0.65, 0.5};

		public:
			RichGraph(Context* context);

			virtual void render() override;

			virtual bool onAction(ActionEvent act) override;

			virtual bool onBeginActivate() override;

			virtual void onEndActivate() override;

			void scale(float32 s);

			void addGraph(std::unique_ptr<SubGraph> graph, std::string label, bool axisX, bool axisY);
	};
}
