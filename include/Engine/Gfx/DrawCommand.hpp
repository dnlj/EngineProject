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

			glm::mat4 mvp; // TODO: find better solution for uniforms
			
			// TODO: Textures
			// TODO: Buffers
			// TODO: Uniforms
	};
}
