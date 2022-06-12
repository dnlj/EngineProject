// Engine
#include <Engine/Gfx/Mesh.hpp>
#include <Engine/Gfx/gfxstate.hpp>
#include <Engine/Camera.hpp>

// Game
#include <Game/systems/AnimSystem.hpp>
#include <Game/comps/ModelComponent.hpp>

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


namespace Game {
	AnimSystem::AnimSystem(SystemArg arg) : System{arg} {
		using namespace Engine::Gfx;

		ent = world.createEntity();

		shaderSkinned = engine.getShaderLoader().get("shaders/mesh");
		shaderStatic = engine.getShaderLoader().get("shaders/mesh_static");

		{
			VertexAttributeDesc attribs[] = {
				{ VertexInput::Position, 3, NumberType::Float32, offsetof(Vertex, pos), false },
				{ VertexInput::BoneIndices, 4, NumberType::UInt8, offsetof(Vertex, bones), false },
				{ VertexInput::BoneWeights, 4, NumberType::Float32, offsetof(Vertex, weights), false },
			};

			layout = engine.getVertexLayoutLoader().get(attribs);
		}

		{
			ModelLoader loader;

			model.skinned = !loader.arm.bones.empty();

			ENGINE_INFO("**** Loaded Model: ", loader.verts.size(), " ", loader.indices.size(), " ", loader.instances.size());

			const auto vbo = engine.getBufferManager().create(loader.verts);
			const auto ebo = engine.getBufferManager().create(loader.indices);

			std::vector<MeshRef> meshes;
			meshes.reserve(loader.meshes.size());
			for (auto& m : loader.meshes) {
				meshes.emplace_back(engine.getMeshManager().create(
					layout,
					vbo, static_cast<uint32>(sizeof(Vertex)),
					ebo, m.offset, m.count
				));
			}

			model.instances.reserve(loader.instances.size());
			for (const auto& inst : loader.instances) {
				model.instances.emplace_back(inst.nodeId, meshes[inst.meshId]);
			}

			// TODO: really arm.bones is the same for all instances of a mesh, its just the offset matrix. Would it make sense to refify armatures?
			model.arm = std::move(loader.arm);
			if (!loader.animations.empty()) {
				animation = std::move(loader.animations[0]);
			}

			// TODO: this should be in part of the mesh/model/ something to desc buffer bindings
			glVertexArrayVertexBuffer(layout->vao, 0, vbo->get(), 0, sizeof(Vertex));
			glVertexArrayElementBuffer(layout->vao, ebo->get());

			// TODO: multiple meshes
			world.addComponent<ModelComponent>(ent, model.skinned ? shaderSkinned : shaderStatic, meshes[0]);
		}

		model.bones.resize(model.arm.bones.size());

		if (!model.bones.empty()) {
			ubo = engine.getBufferManager().create(model.bones.size() * sizeof(model.bones[0]), StorageFlag::DynamicStorage);

			glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo->get()); // Bind index to ubo
			glUniformBlockBinding(shaderSkinned->get(), 0, 1); // Bind uniform block to buffer index
		}
	}

	AnimSystem::~AnimSystem() {
		glDeleteBuffers(1, &cmdbuff);
	}

	void AnimSystem::updateAnim() {
		const auto nodeCount = model.arm.nodes.size();
		const auto tick = fmodf(clock() / 80.0f, animation.duration);

		for (const auto& seq : animation.channels) {
			const auto& interp = seq.interp(tick);
			model.arm.nodes[seq.nodeId].trans = glm::scale(glm::translate(glm::mat4{1.0f}, interp.pos) * glm::mat4_cast(interp.rot), interp.scale);
		}

		for (Engine::Gfx::NodeId ni = 0; ni < nodeCount; ++ni) {
			auto& node = model.arm.nodes[ni];
			if (node.parentId >= 0) {
				node.total = model.arm.nodes[node.parentId].total * node.trans;
			} else {
				node.total = node.trans;
			}

			if (node.boneId >= 0) {
				model.bones[node.boneId] = node.total * model.arm.bones[node.boneId].offset;
			}
		}

		if (ubo) { ubo->setData(model.bones); }
	}

	void AnimSystem::render(const RenderLayer layer) {
		if (layer != RenderLayer::Debug) { return; }

		updateAnim();

		auto vp = glm::ortho<float32>(0, 1920, 0, 1080, -10000, 10000);
		vp = engine.getCamera().getProjection();
		vp *= glm::scale(glm::mat4{1}, glm::vec3{1.0f / pixelsPerMeter});
		//vp *= glm::scale(glm::mat4{1}, glm::vec3{1.0f / 2});

		if (!model.skinned) {
			glUseProgram(shaderStatic->get());

			for (auto& inst : model.instances) {
				// TODO: do we really want static meshes to be animated? surely this should be done on the Entity level if that is the case.
				// TODO: cont. if we want mesh level animation it should probably be rigged? (this seems to be what modeling programs assume)
				const auto& node = model.arm.nodes[inst.nodeId];
				const auto& mesh = inst.mesh;
				const auto mvp = vp * node.total;

				glUniformMatrix4fv(0, 1, GL_FALSE, &mvp[0][0]);
				glBindVertexArray(layout->vao);
				glDrawElements(GL_TRIANGLES, mesh->ecount, GL_UNSIGNED_INT, (const void*)(uintptr_t)(mesh->eoffset * sizeof(GLuint)));
			}
		} else {
			glProgramUniformMatrix4fv(shaderSkinned->get(), 0, 1, GL_FALSE, &vp[0][0]);
			/*
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
			glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, (GLsizei)std::size(commands), 0);*/
		}
	}
}
