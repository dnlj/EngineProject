#pragma once

// Engine
#include <Engine/Gui/Panel.hpp>
#include <Engine/Gfx/Mesh.hpp>
#include <Engine/Math/Math.hpp>
#include <Engine/Gui/Context.hpp>


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

	class Graph : public Panel {
		public: // TODO: private
			std::vector<std::unique_ptr<SubGraph>> graphs;
			glm::vec2 lastDragPos = {};

		public:
			Graph(Context* context) : Panel{context} {
			}

			virtual void render() const override;

			virtual void onAction(ActionEvent act) override;

			virtual void onBeginActivate() override;

			virtual void onEndActivate() override;

			void scale(float32 s);

		private:
	};
}
