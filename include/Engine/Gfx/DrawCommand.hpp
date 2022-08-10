#pragma once


namespace Engine::Gfx {
	class MaterialInstance;
	class Mesh2;

	class DrawCommand {
		public:
			MaterialInstance* material;
			const Mesh2* mesh;

			glm::mat4 mvp; // TODO: find better solution for uniforms

			// TODO: Textures
			// TODO: Buffers
			// TODO: Uniforms
	};
}
