#pragma once


namespace Engine::Gfx {
	class MaterialParams;

	class DrawCommand {
		public:
			// TODO: should we be dealing with pointers to our own types still; `Shader*` etc? We dont want ref types here because of ref counting.
			uint32 program;
			uint32 vao;
			uint32 vbo;
			uint32 vboStride;
			uint32 ebo;
			uint32 ecount;
			uint32 eoffset;

			glm::mat4 mvp; // TODO: find better solution for uniforms

			const MaterialParams* params;
			
			// TODO: Textures
			// TODO: Buffers
			// TODO: Uniforms
	};
}
