// Engine
#include <Engine/Gfx/Context.hpp>
#include <Engine/Gfx/Material.hpp>
#include <Engine/Gfx/Buffer.hpp>
#include <Engine/Gfx/BufferManager.hpp>


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
	Context::Context(BufferManager& bufferManager) {
		matParamsBuffer = bufferManager.create();
	}

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

			if (a.material != b.material) { return a.material < b.material; }

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
			if (active.material != cmd.material) {}
			if (active.vao != cmd.vao) {}
			if (active.vbo != cmd.vbo) {}
			if (active.ebo != cmd.ebo) {}

			constexpr uint32 matParamsBufferIndex = 2;

			const auto prog = cmd.material->material->getShader()->get();
			glUseProgram(prog);
			glBindVertexArray(cmd.vao);

			// TODO: If we always use the same index then we dont ever need to rebind this one right?
			// TODO: cont. Unless we resize the buffer i guess? does it make sense to do that?
			glBindBufferBase(GL_UNIFORM_BUFFER, matParamsBufferIndex, matParamsBuffer->get());
			glUniformBlockBinding(prog, 1, matParamsBufferIndex);

			glVertexArrayVertexBuffer(cmd.vao, 0, cmd.vbo, 0, cmd.vboStride);
			glVertexArrayElementBuffer(cmd.vao, cmd.ebo);
			glUniformMatrix4fv(0, 1, GL_FALSE, &cmd.mvp[0][0]);

			// TODO: use Material::getParametersBlockSize
			if (16 > matParamsBufferSize) {
				matParamsBufferSize = 16;
				matParamsBuffer->alloc(16, StorageFlag::DynamicStorage);
			}
			matParamsBuffer->setData(16, cmd.material->params.data());

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
