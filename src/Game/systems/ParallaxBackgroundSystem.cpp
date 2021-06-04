// GLM
#include <glm/vec2.hpp>

// Game
#include <Game/World.hpp>


namespace Game {
	ParallaxBackgroundSystem::ParallaxBackgroundSystem(SystemArg arg)
		: System{arg} {
		constexpr glm::vec2 rect[] = {
			{1.0f, 1.0f},
			{-1.0f, 1.0f},
			{-1.0f, -1.0f},

			{-1.0f, -1.0f},
			{1.0f, -1.0f},
			{1.0f, 1.0f},
		};

		shader = engine.shaderManager.get("shaders/parallax");
		texture = engine.textureManager.get("assets/large_sprite_test.png");

		glCreateVertexArrays(1, &vao);

		glCreateBuffers(1, &vbo);
		glNamedBufferData(vbo, sizeof(rect), &rect, GL_STATIC_DRAW);
		glVertexArrayVertexBuffer(vao, dataBindingIndex, vbo, 0, sizeof(rect[0]));

		// Vertex attributes
		glEnableVertexArrayAttrib(vao, 0);
		glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(vao, 0, dataBindingIndex);

		// TODO: for per instance data look into glVertexArrayBindingDivisor
	}

	ParallaxBackgroundSystem::~ParallaxBackgroundSystem() {
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
	}

	// TODO: user setting for parallax strength
	void ParallaxBackgroundSystem::render(const RenderLayer layer) {
		if (layer != RenderLayer::Parallax_Background) { return; }

		const auto& cam = engine.camera;

		glBindVertexArray(vao);
		glUseProgram(*shader);

		// TODO: need to add a units per pixel and zoom params to camera so we can pull those.
		// TODO: should be per layer vert attrib - glVertexArrayBindingDivisor
		const glm::vec2 scale = glm::vec2{texture->size} * (2.0f / glm::vec2{engine.camera.getScreenSize()});
		glUniform2fv(0, 1, &scale[0]);

		glBindTextureUnit(0, texture->tex.get());
		glUniform1i(1, 0);


		// TODO: should be vert attrib - glVertexArrayBindingDivisor
		const GLfloat scrollScale = 0.5f;
		const GLfloat worldToTexCoords = static_cast<GLfloat>(pixelsPerMeter) / texture->size.x;
		const glm::vec2 pos = engine.camera.getPosition();
		glUniform1f(2, pos.x * worldToTexCoords * scrollScale);

		// Draw
		glDrawArrays(GL_TRIANGLES, 0, 6);


	}
}
