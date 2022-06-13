#pragma once


namespace Engine::Gfx {
	class Shader;
	class MaterialParams;

	class DrawCommand {
		public:
			// TODO: should we be dealing with pointers to our own types still; `Shader*` etc? We dont want ref types here because of ref counting.

			// TODO: we should probably just pass around the MaterialInstance* instead of shader + params
			const Shader* shader;
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
