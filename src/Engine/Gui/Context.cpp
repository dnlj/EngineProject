// Engine
#include <Engine/Gui/Context.hpp>


namespace Engine::Gui {
	Context::Context(Engine::EngineInstance& engine) {
		shader = engine.shaderManager.get("shaders/gui");
		view = engine.camera.getScreenSize(); // TODO: should update when resized

		glCreateVertexArrays(1, &vao);
		glCreateBuffers(1, &vbo);
		glVertexArrayVertexBuffer(vao, vertBindingIndex, vbo, 0, sizeof(verts[0]));

		// Vertex
		glEnableVertexArrayAttrib(vao, 0);
		glVertexArrayAttribBinding(vao, 0, vertBindingIndex);
		glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, GL_FALSE, 0);

		ENGINE_LOG("View: ", view.x, " ", view.y);

		root = new Panel{};
		root->setPos({25, 50});
		root->setSize({512, 256});

		{
			auto child = root->addChild(new Panel{});
			child->setPos({5, 5});
			child->setSize({64, 64});
		}
		
		{
			auto child = root->addChild(new Panel{});
			child->setPos({20, 25});
			child->setSize({32, 64});
		}
	}

	Context::~Context() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		delete root;
	}

	void Context::render() {
		const Panel* curr = root;
		offset = {};
		//int depth = 0; // TODO: rm

		// Breadth first traversal
		while (true) {
			while (curr) {
				if (curr->firstChild) {
					bfsNext.emplace_back(
						offset + curr->getPos(),
						curr->firstChild
					);
				}

				curr->render(*this);
				curr = curr->nextSibling;
			}

			if (bfsCurr.empty()) {
				bfsCurr.swap(bfsNext);
				if (bfsCurr.empty()) { break; }
				// ENGINE_LOG("GUI: depth++ ", ++depth);
			}

			const auto& back = bfsCurr.back();
			curr = back.panel;
			offset = back.offset;
			bfsCurr.pop_back();
		}

		{
			const auto size = verts.size() * sizeof(verts[0]);
			if (size > vboCapacity) {
				vboCapacity = static_cast<GLsizei>(verts.capacity() * sizeof(verts[0]));
				glNamedBufferData(vbo, vboCapacity, nullptr, GL_DYNAMIC_DRAW);
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
		verts.push_back(offset + pos);
		verts.push_back(offset + pos + glm::vec2{0, size.y});
		verts.push_back(offset + pos + size);

		verts.push_back(offset + pos + size);
		verts.push_back(offset + pos + glm::vec2{size.x, 0});
		verts.push_back(offset + pos);
	}

	bool Context::onMouse(const Engine::Input::InputEvent event) {
		// ENGINE_LOG("onMouse: ", event.state.value, " @ ", Engine::Clock::Seconds{event.time.time_since_epoch()}.count());
		return false;
	}

	bool Context::onMouseMove(const Engine::Input::InputEvent event) {
		// ENGINE_LOG("onMouseMove: ", event.state.value, " @ ", Engine::Clock::Seconds{event.time.time_since_epoch()}.count());
		return false;
	}

	bool Context::onMouseWheel(const Engine::Input::InputEvent event) {
		// ENGINE_LOG("onMouseWheel: ", event.state.value, " @ ", Engine::Clock::Seconds{event.time.time_since_epoch()}.count());
		return false;
	}

	bool Context::onKey(const Engine::Input::InputEvent event) {
		// ENGINE_LOG("onKey: ", event.state.value, " @ ", Engine::Clock::Seconds{event.time.time_since_epoch()}.count());
		return false;
	}

	bool Context::onChar(const wchar_t ch) {
		// ENGINE_LOG("onChar: ", (int)ch);
		return false;
	}

	void Context::onResize(const int32 w, const int32 h) {
		// ENGINE_LOG("onResize: ", w, " ", h);
		view = {w, h};
	}

	void Context::onFocus(const bool has) {
		// ENGINE_LOG("onFocus: ", has);
	}

}
