// Engine
#include <Engine/Gfx/Mesh.hpp>

// Game
#include <Game/systems/AnimSystem.hpp>

namespace Game {
	AnimSystem::AnimSystem(SystemArg arg) : System{arg} {
		engine.shaderManager.add("shaders/mesh");
		shader = engine.shaderManager.get("shaders/mesh");

		{
			Engine::Gfx::ModelLoader loader;
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
		glUniformBlockBinding(shader->get(), 0, 1); // Bind uniform block to buffer index
	}

	AnimSystem::~AnimSystem() {
		glDeleteBuffers(1, &ubo);
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

		auto mvp = glm::ortho<float32>(0, 1920, 0, 1080, -10000, 10000);
		mvp = engine.camera.getProjection();
		//mvp *= glm::scale(glm::mat4{1}, glm::vec3{1.0f / pixelsPerMeter});
		mvp *= glm::scale(glm::mat4{1}, glm::vec3{1.0f / 4});

		glUseProgram(shader->get());
		glUniformMatrix4fv(0, 1, GL_FALSE, &mvp[0][0]);

		test.draw();
	}
}
