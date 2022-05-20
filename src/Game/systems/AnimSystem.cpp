// Engine
#include <Engine/Gfx/Mesh.hpp>

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
		engine.shaderManager.add("shaders/mesh");
		engine.shaderManager.add("shaders/mesh_static");
		shaderSkinned = engine.shaderManager.get("shaders/mesh");
		shaderStatic = engine.shaderManager.get("shaders/mesh_static");

		{
			Engine::Gfx::ModelLoader loader;

			ENGINE_INFO("**** Loaded Model: ", loader.verts.size(), " ", loader.indices.size());
			meshes = std::move(loader.meshes);
			instances = std::move(loader.instances);
			
			arm = std::move(loader.arm);
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

		bonesFinal.resize(arm.bones.size());

		glCreateBuffers(1, &ubo);
		glNamedBufferData(ubo, bonesFinal.size() * sizeof(bonesFinal[0]), nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo); // Bind index to ubo
		glUniformBlockBinding(shaderSkinned->get(), 0, 1); // Bind uniform block to buffer index
	}

	AnimSystem::~AnimSystem() {
		glDeleteBuffers(1, &ubo);
		glDeleteBuffers(1, &cmdbuff);
	}

	void AnimSystem::updateAnim() {
		const auto nodeCount = arm.nodes.size();
		const auto tick = fmodf(clock() / 100.0f, animation.duration);

		for (const auto& seq : animation.channels) {
			const auto& interp = seq.interp(tick);
			arm.nodes[seq.nodeId].trans = glm::scale(glm::translate(glm::mat4{1.0f}, interp.pos) * glm::mat4_cast(interp.rot), interp.scale);
		}

		for (Engine::Gfx::NodeId ni = 0; ni < nodeCount; ++ni) {
			auto& node = arm.nodes[ni];
			if (node.parentId >= 0) {
				node.total = arm.nodes[node.parentId].total * node.trans;
			} else {
				node.total = node.trans;
			}

			if (node.boneId >= 0) {
				bonesFinal[node.boneId] = node.total * arm.bones[node.boneId].offset;
			}
		}

		glNamedBufferSubData(ubo, 0, bonesFinal.size() * sizeof(bonesFinal[0]), bonesFinal.data());
	}

	void AnimSystem::render(const RenderLayer layer) {
		if (layer != RenderLayer::Debug) { return; }

		updateAnim();

		auto vp = glm::ortho<float32>(0, 1920, 0, 1080, -10000, 10000);
		vp = engine.camera.getProjection();
		//vp *= glm::scale(glm::mat4{1}, glm::vec3{1.0f / pixelsPerMeter});
		vp *= glm::scale(glm::mat4{1}, glm::vec3{1.0f / 2});

		//glUseProgram(shaderSkinned->get());
		//glUniformMatrix4fv(0, 1, GL_FALSE, &mvp[0][0]);

		//test.draw();

		//static std::vector<DrawElementsIndirectCommand> commands; // TODO: rm temp

		for (auto& inst : instances) {
			const auto& node = arm.nodes[inst.nodeId];
			const auto& mesh = meshes[inst.meshId];

			const auto mvp = vp * node.total;

			// TODO: the problem is we only have one anim and blender splits it into 4 anims for some reason instead of 1 or two
			glUseProgram(shaderStatic->get());
			glUniformMatrix4fv(0, 1, GL_FALSE, &mvp[0][0]);

			glBindVertexArray(test.getVAO());
			glDrawElements(GL_TRIANGLES, mesh.count, GL_UNSIGNED_INT, (const void*)(uintptr_t)(mesh.offset * sizeof(GLuint)));
		}

		/*if (cmdbuff == 0) {
			ENGINE_DEBUG_ASSERT(meshes.size() == 2);
			for (const auto& mesh : meshes) {
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

		glBindVertexArray(test.getVAO());
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, cmdbuff);
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, (GLsizei)std::size(commands), 0);*/
	}
}
