// Engine
#include <Engine/Gfx/Context.hpp>


namespace {
	typedef struct {
        uint32_t  count;
        uint32_t  instanceCount;
        uint32_t  firstIndex;
         int32_t  baseVertex;
        uint32_t  baseInstance;
    } DrawElementsIndirectCommand;
	static_assert(sizeof(DrawElementsIndirectCommand) == 4 * 5, "glMultiDrawElementsIndirect requires that the indirect structure is tightly packed if using a stride of zero.");
}


namespace Engine::Gfx {
	void Context::render() {
		// TODO: move to own function
		std::ranges::sort(cmds, [](const DrawCommand& a, const DrawCommand& b) noexcept {
			// Sorted based on approximate cost from:
			// Steam Dev Days: Beyond Porting
			// GDC 2014: Approaching Zero Driver Overhead
			// ---------------------------------------------
			// Render target
			// Program
			// ROP (blending, depth, stencil, etc.)
			// Texture sampler uniform update
			// Texture bindings
			// Vertex format
			// UBO bindings
			// Vertex bindings
			// Uniform updates
			// ---------------------------------------------

			if (a.program != b.program) { return a.program < b.program; }

			// TODO: textures. Not sure how we want to do this.
			//
			// If texture sampler uniforms are really more expensive than binding a texture should we just have them fixed
			// and swap the bindings around (at that point they dont even need to be uniforms?)?
			//
			// If that is not true then we should have an extra layer of indirection to
			// remap uniform->binding index if we already have a texture bound.
			//
			// Bindless sparse array textures are also a thing. Maybe look into that.
			// 
			//if (a.textures[0].texture != b.textures[0].texture) { return a.textures[0].texture > b.textures[0].texture; }

			if (a.vao != b.vao) { return a.vao < b.vao; }
			// TODO: UBO
			if (a.vbo != b.vbo) { return a.vbo < b.vbo; }
			if (a.ebo != b.ebo) { return a.ebo < b.ebo; }
			// TODO: buffers
			// TODO: uniforms

			return false;
		});

		for (const auto& cmd : cmds) {
			// TODO: rework to use glMultiDrawElementsIndirect and uniforms array buffers
			if (active.program != cmd.program) {}
			if (active.vao != cmd.vao) {}
			if (active.vbo != cmd.vbo) {}
			if (active.ebo != cmd.ebo) {}

			glUseProgram(cmd.program);
			glBindVertexArray(cmd.vao);
			glVertexArrayVertexBuffer(cmd.vao, 0, cmd.vbo, 0, cmd.vboStride);
			glVertexArrayElementBuffer(cmd.vao, cmd.ebo);

			glDrawElementsInstancedBaseVertexBaseInstance(
				GL_TRIANGLES,
				cmd.ecount,
				GL_UNSIGNED_INT,
				// TODO: note that when we change this to use glMultiDrawElementsIndirect that the offset changes from bytes to indices
				reinterpret_cast<const void*>(static_cast<uintptr_t>(cmd.eoffset * sizeof(uint32))),
				1, 0, 0
			);
		}

		cmds.clear();
	}
}
