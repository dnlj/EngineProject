#pragma once

// Engine
#include <Engine/StaticVector.hpp>


namespace Engine::Gfx {
	class MaterialInstance;
	class Mesh2;
	class Buffer;

	class DrawCommand {
		public:
			struct BufferBinding {
				uint32 index;
				const Buffer* buff;
			};

		public:
			MaterialInstance* material;
			const Mesh2* mesh;

			glm::mat4 mvp; // TODO: find better solution for uniforms

			/** Uniform block bindings */
			StaticVector<BufferBinding, 4> blockBindings; // TODO: not sure the best way to handle this. (and uniforms in general)

			// TODO: Textures
			// TODO: Buffers
			// TODO: Uniforms
	};
}
