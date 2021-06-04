// GLM
#include <glm/vec2.hpp>

// Game
#include <Game/World.hpp>


namespace Game {
	ParallaxBackgroundSystem::ParallaxBackgroundSystem(SystemArg arg)
		: System{arg} {

		shader = engine.shaderManager.get("shaders/parallax");
		texture = engine.textureManager.get("assets/para_test_0.png");

		glCreateVertexArrays(1, &vao);

		glCreateBuffers(1, &vbo);
		glNamedBufferData(vbo, sizeof(rect) + sizeof(InstData), nullptr, GL_DYNAMIC_DRAW);
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

	// TODO: user setting for parallax strength
	void ParallaxBackgroundSystem::render(const RenderLayer layer) {
		if (layer != RenderLayer::Parallax_Background) { return; }

		const auto& cam = engine.camera;
		const GLfloat scrollScale = 0.5f;
		const GLfloat worldToTexCoords = static_cast<GLfloat>(pixelsPerMeter) / texture->size.x;
		const glm::vec2 pos = engine.camera.getPosition();

		InstData data;
		data.scale = glm::vec2{texture->size} * (2.0f / glm::vec2{engine.camera.getScreenSize()});
		data.xoff = pos.x * worldToTexCoords * scrollScale;

		glNamedBufferSubData(vbo, sizeof(rect), sizeof(InstData), &data);

		glBindVertexArray(vao);
		glUseProgram(*shader);

		glBindTextureUnit(0, texture->tex.get());
		glUniform1i(0, 0);

		// Draw
		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
	}
}
