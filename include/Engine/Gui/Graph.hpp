#pragma once

// Engine
#include <Engine/Gui/Panel.hpp>
#include <Engine/Graphics/Mesh.hpp>


namespace Engine::Gui {
	class Graph : public Panel {
		private:
			struct Vertex {
				glm::vec2 pos;
				glm::vec4 color;
			};

			std::vector<Vertex> meshData;
			std::vector<glm::vec2> points;

		public:
			Graph(Context* context) : Panel{context} {
				Graphics::VertexFormat<2> format = {
					.stride = sizeof(Vertex),
					.attributes = {
						// TODO: location depends on shader right?
						{.location = 1, .size = 2, .type = GL_FLOAT, .offset = offsetof(Vertex, pos)},
						{.location = 2, .size = 4, .type = GL_FLOAT, .offset = offsetof(Vertex, color)},
					},
				};
			}

			// TODO: do we want an getPoint function like imgui or do we want our own addPoint function?
			// The benefit of doing an addPoint function would be we know exactly when and what data
			// is updated vs polling and doing a full rebuild every frame.
			// The downside is that we have to store duplicate data.
			//
			// I think i want to to the addPoint way. how to handle culling data?

		private:
			void updateMesh() {
			}

	};
}
