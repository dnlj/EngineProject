#pragma once

// Engine
#include <Engine/Gfx/resources.hpp>


namespace Engine::Gfx {
	class Mesh2 {
		public:
			/**
			 * Vertex buffer.
			 * Vertex layouts could actually draw from multiple buffers (binding points), but atm we never actually do that.
			 * @see glVertexArrayVertexBuffer
			 * @see glVertexArrayAttribBinding
			 */
			BufferRef vbuff;
			//MaterialRef mat;

			/** Vertex buffer layout */
			VertexAttributeLayoutRef layout;
			// TODO: still need to setup buffers - glVertexArrayVertexBuffer(s)

			/** Element buffer */
			BufferRef ebuff;

			/** Element buffer offset */
			uint32 offset = 0;
			
			/** Element buffer count */
			uint32 count = 0;

			// Not needed currently because we only use one vertex buffer.
			// If in the future we need to draw from multiple vertex buffers (binding points) we will need to be able to describe buffer -> binding point mappings
			// vector<pair<int, int>> bufferToBindingMapping;
	};
}
