// Engine
#include <Engine/Gui/Context.hpp>


namespace Engine::Gui {
	Context::Context(Engine::EngineInstance& engine) {
		shader = engine.shaderManager.get("shaders/gui");
		view = engine.camera.getScreenSize();

		glCreateVertexArrays(1, &vao);

		glCreateBuffers(1, &vbo);
		glVertexArrayVertexBuffer(vao, vertBindingIndex, vbo, 0, sizeof(verts[0]));

		// Vertex attributes
		glEnableVertexArrayAttrib(vao, 0);
		glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(vao, 0, vertBindingIndex);
	}

	Context::~Context() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
	}

	void Context::render() {
		{
			const auto size = verts.size() * sizeof(verts[0]);
			if (size > vboCapacity) {
				vboCapacity = static_cast<GLsizei>(verts.capacity() * sizeof(verts[0]));
				glNamedBufferData(vbo, vboCapacity, nullptr, GL_DYNAMIC_DRAW);
				ENGINE_LOG("Resize: ", vboCapacity);
			}
			glNamedBufferSubData(vbo, 0, size, verts.data());
		}

		glBindVertexArray(vao);
		glUseProgram(*shader);
		glUniform2fv(0, 1, &view.x);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(verts.size()));

		glDisable(GL_BLEND);
		verts.clear();
	}

	void Context::addRect(const glm::vec2 pos, const glm::vec2 size) {
		verts.push_back(pos);
		verts.push_back(pos + glm::vec2{0, size.y});
		verts.push_back(pos + size);

		verts.push_back(pos + size);
		verts.push_back(pos + glm::vec2{size.x, 0});
		verts.push_back(pos);
	}
}
