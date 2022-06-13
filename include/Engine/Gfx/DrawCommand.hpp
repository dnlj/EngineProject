#pragma once


namespace Engine::Gfx {
	class MaterialInstance;

	class DrawCommand {
		public:
			const MaterialInstance* material;

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
