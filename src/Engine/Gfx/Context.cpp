// Engine
#include <Engine/Gfx/Context.hpp>
#include <Engine/Gfx/Material.hpp>
#include <Engine/Gfx/Buffer.hpp>
#include <Engine/Gfx/BufferManager.hpp>
#include <Engine/Gfx/Mesh2.hpp>
#include <Engine/Gfx/VertexAttributeLayout.hpp>
#include <Engine/Gfx/ActiveTextureCache.hpp>


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
	Context::Context(BufferManager& bufferManager, TextureLoader& textureLoader) {
		matParamsBuffer = bufferManager.create();
		errTexture = textureLoader.getErrorTexture2D();

		// TODO: this really only fixes it for 2d textures, would need error textures for each target to fully fix this.
		for (uint32 i = 0; i < maxActiveTextures; ++i) {
			glBindTextureUnit(i, errTexture->tex.get());
		}
	}

	void Context::render() {
		/*
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
			// probably want to sort be ref count then textureid so that as a heuristic.
			// In theory we should be able to find the optimal ordering, just not sure what that is called atm.
			//if (a.textures[0].texture != b.textures[0].texture) { return a.textures[0].texture > b.textures[0].texture; }

			if (a.vao != b.vao) { return a.vao < b.vao; }
			// TODO: UBO
			if (a.vbo != b.vbo) { return a.vbo < b.vbo; }
			if (a.ebo != b.ebo) { return a.ebo < b.ebo; }
			// TODO: buffers
			// TODO: uniforms

			return false;
		});*/

		{
			// TODO: in the future we could have this persist between draws, but at the
			// ^^^^: moment we have other draw systems that likely change textures and invalidate
			// ^^^^: our state here.
			texCache.clear();
			memset(texActive, 0, sizeof(texActive));
		}

		for (const auto& cmd : cmds) {
			auto& mat = cmd.material;
			auto& mesh = cmd.mesh;

			const auto program = mat->base->getShader()->get();
			const auto vao = mesh->layout->get();

			glUseProgram(program);
			glBindVertexArray(vao);
			
			// TODO: read from material. See src/engine/gfx/material.cpp - we already have code for this
			constexpr uint32 matParamsBufferIndex = 2;

			// TODO: If we always use the same index then we dont ever need to rebind this one right?
			// TODO: cont. Unless we resize the buffer i guess? does it make sense to do that?
			glBindBufferBase(GL_UNIFORM_BUFFER, matParamsBufferIndex, matParamsBuffer->get());

			// TODO: also probably diff with active binds so we only bind when needed - should be elimiated when we get draw sorting working though so probably not worth fixing atm
			// TODO: use glBindBuffersRange instead to bind all with one cmd. Need to redo DrawCommand::blockBindings to use SoA layout.
			for (const auto& bind : cmd.uboBindings) {
				ENGINE_DEBUG_ASSERT(bind.size > 0, "Invalid ubo binding size");
				glBindBufferRange(GL_UNIFORM_BUFFER, bind.index, bind.buff->get(), bind.offset, bind.size);
			}

			for (const auto& bind : cmd.vboBindings) {
				ENGINE_DEBUG_ASSERT(bind.size > 0, "Invalid vbo binding size");
				glVertexArrayVertexBuffer(vao, bind.index, bind.buff->get(), bind.offset, bind.size);
			}

			glVertexArrayElementBuffer(vao, mesh->ebuff->get());

			const auto matParamsSize = mat->base->getParameterDescription().blockSize;
			if (matParamsSize > matParamsBufferSize) {
				matParamsBufferSize = matParamsSize;
				matParamsBuffer->alloc(matParamsSize, StorageFlag::DynamicStorage);
				// TODO: we need rebind the buffer since we realloc
			}

			// TODO: when we go to multi-indirect rendering we should be able to have
			// ^^^^: multiple draw batches worth of unifrom data in one buffer instead of per
			// ^^^^: batch re-upload
			{
				texCache.use(*mat);

				{
					auto activeCurr = texActive;
					auto cacheCurr = texCache.begin();

					for (uint32 i = 0; i < maxActiveTextures; ++i, ++activeCurr, ++cacheCurr) {
						if (*activeCurr != cacheCurr->tex) {
							*activeCurr = cacheCurr->tex;
							glBindTextureUnit(i, activeCurr->get());
						}
					}
				}

				for (const auto& tex : mat->getTextures()) {
					mat->set<uint32>(tex.first, texCache.get(tex.second->tex));
				}

				// TODO: maybe bindless? makes this kind of thing a lot easier
				// TODO: need to set textures uniform
				//glProgramUniform1iv(program, glGetUniformLocation(program, "textures"), (GLsizei)std::size(texIndices), texIndices);

			}
			matParamsBuffer->setData(mat->data(), matParamsSize);

			// TODO: batch material and instance parameters into same buffer object (glBindBufferRange)
			// TODO: we really need to do uniform introspection for instance data like we do for material uniforms so we can group uniform data.

			glDrawElementsInstancedBaseVertexBaseInstance(
				GL_TRIANGLES,
				mesh->ecount,
				GL_UNSIGNED_INT,
				// TODO: note that when we change this to use glMultiDrawElementsIndirect that the offset changes from bytes to indices
				reinterpret_cast<const void*>(static_cast<uintptr_t>(mesh->eoffset * sizeof(uint32))),
				1, 0, cmd.baseInstance
			);
		}

		/* Example code i used to have in AnimSystem using multidraw. Note that this doesnt handle uniforms or similar.
		glUseProgram(shaderSkinned->get());
		glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);

		static std::vector<DrawElementsIndirectCommand> commands; // TODO: rm temp

		if (cmdbuff == 0) {
			for (const auto& inst : model.instances) {
				const auto& mesh = inst.mesh;
				commands.push_back({
					.count = mesh->ecount,
					.instanceCount = 1,
					.firstIndex = mesh->eoffset,
					.baseVertex = 0,
					.baseInstance = 0,
				});
			}

			glCreateBuffers(1, &cmdbuff);
			glNamedBufferStorage(cmdbuff, commands.size() * sizeof(commands[0]), nullptr, GL_DYNAMIC_STORAGE_BIT);
			glNamedBufferSubData(cmdbuff, 0, commands.size() * sizeof(commands[0]), std::data(commands));
		}

		glBindVertexArray(layout->vao);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, cmdbuff);
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, (GLsizei)std::size(commands), 0);
		*/

		cmds.clear();
	}
}
