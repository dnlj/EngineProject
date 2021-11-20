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
			bool rebuild = false;
			std::vector<GraphVertex> data;
			Gfx::Mesh mesh;

		public:
			virtual void addPoint(glm::vec2 p) = 0;
			Gfx::Mesh& getMesh() {
				if (rebuild) {
					mesh.setVertexData(Gfx::Primitive::TriangleStrip, data);
				}
				return mesh;
			}
	};

	class AreaGraph : public SubGraph {
		public:
			virtual void addPoint(glm::vec2 p) {
				rebuild = true;
				data.push_back({
					.pos = {p.x, 0},
					.color = {1,0,0,1},
				});
				data.push_back({
					.pos = p,
					.color = {1,0,0,1},
				});
			};
	};

	class Graph : public Panel {
		private:

		public:
			Graph(Context* context) : Panel{context} {
				//Graphics::VertexFormat<2> format = {
				//	.stride = sizeof(GraphVertex),
				//	.attributes = {
				//		// TODO: location depends on shader right?
				//		{.location = 1, .size = 2, .type = GL_FLOAT, .offset = offsetof(GraphVertex, pos)},
				//		{.location = 2, .size = 4, .type = GL_FLOAT, .offset = offsetof(GraphVertex, color)},
				//	},
				//};
			}

			// TODO: do we want an getPoint function like imgui or do we want our own addPoint function?
			// The benefit of doing an addPoint function would be we know exactly when and what data
			// is updated vs polling and doing a full rebuild every frame.
			// The downside is that we have to store duplicate data.
			//
			// I think i want to to the addPoint way. how to handle culling data?

		private:
	};
}
