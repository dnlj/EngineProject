// Engine
#include <Engine/Gfx/Mesh.hpp>
#include <Engine/Gfx/gfxstate.hpp>

// Game
#include <Game/systems/AnimSystem.hpp>

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

		engine.shaderManager.add("shaders/mesh");
		engine.shaderManager.add("shaders/mesh_static");
		shaderSkinned = engine.shaderManager.get("shaders/mesh");
		shaderStatic = engine.shaderManager.get("shaders/mesh_static");

		{
			ModelLoader loader;

			model.skinned = true; // TODO: should be informed the loader

			ENGINE_INFO("**** Loaded Model: ", loader.verts.size(), " ", loader.indices.size());
			model.meshes = std::move(loader.meshes);
			model.instances = std::move(loader.instances);
			
			model.arm = std::move(loader.arm);
			if (!loader.animations.empty()) {
				animation = std::move(loader.animations[0]);
			}

			using NumberType = Engine::Gfx::NumberType;
			test.setFormat(Engine::Gfx::VertexFormat<3>{sizeof(Engine::Gfx::Vertex), {
				{ .location = 0, .size = 3, .type = NumberType::Float32, .offset = offsetof(Engine::Gfx::Vertex, pos) },
				{ .location = 1, .size = 4, .type = NumberType::UInt8, .offset = offsetof(Engine::Gfx::Vertex, bones) },
				{ .location = 2, .size = 4, .type = NumberType::Float32, .offset = offsetof(Engine::Gfx::Vertex, weights) },
			}});

			test.setVertexData(Engine::Gfx::Primitive::Triangles, loader.verts);
			test.setElementData(loader.indices);

		}

		model.bones.resize(model.arm.bones.size());

		ubo.alloc(model.bones.size() * sizeof(model.bones[0]), StorageFlag::DynamicStorage);

		glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo.get()); // Bind index to ubo
		glUniformBlockBinding(shaderSkinned->get(), 0, 1); // Bind uniform block to buffer index

		{
			VertexAttributeDesc attribs[] = {
				{ VertexInput::Position, 3, NumberType::Float32, offsetof(Vertex, pos), false },
				{ VertexInput::BoneIndices, 4, NumberType::UInt8, offsetof(Vertex, bones), false },
				{ VertexInput::BoneWeights, 4, NumberType::Float32, offsetof(Vertex, weights), false },
			};

			layout = engine.vertexLayoutLoader.get(attribs);

			glVertexArrayVertexBuffer(layout->vao, 0, test.getVBO(), 0, sizeof(Vertex));
			glVertexArrayElementBuffer(layout->vao, test.getEBO());

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

		ubo.setData(model.bones);
	}

	void AnimSystem::render(const RenderLayer layer) {
		if (layer != RenderLayer::Debug) { return; }

		updateAnim();

		auto vp = glm::ortho<float32>(0, 1920, 0, 1080, -10000, 10000);
		vp = engine.camera.getProjection();
		vp *= glm::scale(glm::mat4{1}, glm::vec3{1.0f / pixelsPerMeter});
		//vp *= glm::scale(glm::mat4{1}, glm::vec3{1.0f / 2});

		if (!model.skinned) {
			glUseProgram(shaderStatic->get());

			for (auto& inst : model.instances) {
				// TODO: do we really want static meshes to be animated? surely this should be done on the Entity level if that is the case.
				// TODO: cont. if we want mesh level animation it should probably be rigged? (this seems to be what modeling programs assume)
				const auto& node = model.arm.nodes[inst.nodeId];
				const auto& mesh = model.meshes[inst.meshId];
				const auto mvp = vp * node.total;

				glUniformMatrix4fv(0, 1, GL_FALSE, &mvp[0][0]);
				glBindVertexArray(layout->vao);
				glDrawElements(GL_TRIANGLES, mesh.count, GL_UNSIGNED_INT, (const void*)(uintptr_t)(mesh.offset * sizeof(GLuint)));
			}
		} else {
			glUseProgram(shaderSkinned->get());
			glUniformMatrix4fv(0, 1, GL_FALSE, &vp[0][0]);

			static std::vector<DrawElementsIndirectCommand> commands; // TODO: rm temp

			if (cmdbuff == 0) {
				for (const auto& inst : model.instances) {
					const auto& mesh = model.meshes[inst.meshId];
					commands.push_back({
						.count = mesh.count,
						.instanceCount = 1,
						.firstIndex = mesh.offset,
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
		}
	}
}
