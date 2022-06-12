#pragma once


namespace Engine::Gfx {
	class DrawCommand {
		public:
			uint32 program;
			uint32 vao;
			uint32 vbo;
			uint32 vboStride;
			uint32 ebo;
			uint32 ecount;
			uint32 eoffset;

			struct TextureBinding {
				uint32 texture;
				uint32 binding;
			} textures[4];

			struct BufferBinding {
				uint32 type; // UBO, SSBO. See glBindBufferRange
				uint32 buffer;
				uint32 binding;
			} buffers[4];

			struct UniformData {
				// TODO: ????
			} uniforms[1];
	};
}
