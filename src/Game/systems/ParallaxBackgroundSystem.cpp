// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/Camera.hpp>
#include <Engine/Gfx/TextureLoader.hpp>
#include <Engine/Gfx/ShaderLoader.hpp>

// Game
#include <Game/World.hpp>
#include <Game/systems/ParallaxBackgroundSystem.hpp>


namespace Game {
	ParallaxBackgroundSystem::ParallaxBackgroundSystem(SystemArg arg)
		: System{arg} {

		shader = engine.getShaderLoader().get("shaders/parallax");
		
		layers.push_back({
			.texture = engine.getTextureLoader().get("assets/para_test_2.png"),
			.speedScale = 0.05f,
		});

		layers.push_back({
			.texture = engine.getTextureLoader().get("assets/para_test_1.png"),
			.speedScale = 0.15f,
		});

		layers.push_back({
			.texture = engine.getTextureLoader().get("assets/para_test_0.png"),
			.speedScale = 0.3f,
		});
		
		//layers.push_back({
		//	.texture = engine.textureManager.get("assets/large_sprite_test.png"),
		//	.speedScale = 1.0f,
		//});

		instData.resize(layers.size());

		glCreateVertexArrays(1, &vao);

		glCreateBuffers(1, &vbo);
		glNamedBufferData(vbo, sizeof(rect) + instData.size() * sizeof(InstData), nullptr, GL_DYNAMIC_DRAW);
		glNamedBufferSubData(vbo, 0, sizeof(rect), &rect);
		glVertexArrayVertexBuffer(vao, rectBindingIndex, vbo, 0, sizeof(rect[0]));
		glVertexArrayVertexBuffer(vao, instBindingIndex, vbo, sizeof(rect), sizeof(InstData));

		// Vertex attributes
		glEnableVertexArrayAttrib(vao, 0);
		glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(vao, 0, rectBindingIndex);

		// Instance attributes
		glVertexArrayBindingDivisor(vao, instBindingIndex, 1);

		glEnableVertexArrayAttrib(vao, 1); // Scale
		glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(InstData, scale));
		glVertexArrayAttribBinding(vao, 1, instBindingIndex);

		glEnableVertexArrayAttrib(vao, 2); // X Offset
		glVertexArrayAttribFormat(vao, 2, 1, GL_FLOAT, GL_FALSE, offsetof(InstData, xoff));
		glVertexArrayAttribBinding(vao, 2, instBindingIndex);
	}

	ParallaxBackgroundSystem::~ParallaxBackgroundSystem() {
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
	}

	void ParallaxBackgroundSystem::update(const float32 dt) {
		const auto size = instData.size();
		auto& cam = engine.getCamera();

		for (int i = 0; i < size; ++i) {
			auto& inst = instData[i];
			const auto& layer = layers[i];

			constexpr GLfloat strength = 1.0f; // TODO: user setting for parallax strength
			const GLfloat worldToTexCoords = static_cast<GLfloat>(pixelsPerMeter) / layer.texture->size.x;
			const glm::vec2 pos = cam.getPosition();

			inst.scale = glm::vec2{layer.texture->size} * (2.0f / glm::vec2{cam.getScreenSize()});
			inst.xoff = pos.x * worldToTexCoords * layer.speedScale * strength;
		}

		glNamedBufferSubData(vbo, sizeof(rect), size * sizeof(InstData), instData.data());
	}

	void ParallaxBackgroundSystem::render(const RenderLayer layer) {
		if (layer != RenderLayer::Parallax_Background) { return; }

		glBindVertexArray(vao);
		glUseProgram(shader->get());
		glUniform1i(0, 0);

		for (int i = 0; const auto& l : layers) {
			glBindTextureUnit(0, l.texture->tex.get());
			glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, i);
			++i;
		}
	}
}
