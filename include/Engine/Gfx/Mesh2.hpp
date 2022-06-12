#pragma once

// Engine
#include <Engine/Gfx/resources.hpp>


namespace Engine::Gfx {
	class Mesh2 {
		public:
			/** Vertex buffer layout */
			VertexAttributeLayoutRef layout;

			/**
			 * Vertex buffer.
			 * Vertex layouts could actually draw from multiple buffers (binding points), but atm we never actually do that.
			 * @see glVertexArrayVertexBuffer(s)
			 * @see glVertexArrayAttribBinding
			 */
			BufferRef vbuff;

			/** Vertex stride */
			uint32 vstride = 0;

			/** Element buffer */
			BufferRef ebuff;

			/** Element buffer offset */
			uint32 eoffset = 0;
			
			/** Element buffer count */
			uint32 ecount = 0;

			// Not needed currently because we only use one vertex buffer.
			// If in the future we need to draw from multiple vertex buffers (binding points) we will need to be able to describe buffer -> binding point mappings
			// vector<pair<int, int>> bufferToBindingMapping;
	};
}
