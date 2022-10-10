#pragma once

// Engine
#include <Engine/StaticVector.hpp>

namespace Engine::Gfx {
	class MaterialInstance;
	class Mesh;
	class Buffer;

	class DrawCommand {
		public:
			// We def our own struct here instead of using the normal BufferBinding so we dont inc/dec a BufferRef constantly while drawing.
			struct BufferBinding {
				const Buffer* buff;
				uint16 index;
				uint16 offset;
				uint16 size;
			};

		public:
			MaterialInstance* material;
			const Mesh* mesh;
			uint32 baseInstance;

			StaticVector<BufferBinding, 4> uboBindings;
			StaticVector<BufferBinding, 4> vboBindings;

			// TODO: Textures
			// TODO: Buffers
			// TODO: Uniforms
	};
}
