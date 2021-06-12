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
		glVertexArrayAttribFormat(vao, 0, 4, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));

		glEnableVertexArrayAttrib(vao, 1);
		glVertexArrayAttribBinding(vao, 1, vertBindingIndex);
		glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));

		root = new Panel{};
		root->setPos({25, 50});
		root->setSize({512, 256});

		{
			auto child = root->addChild(new Panel{});
			child->setPos({5, 5});
			child->setSize({64, 64});

			auto childChild = child->addChild(new Panel{});
			childChild->setPos({5, 5});
			childChild->setSize({32, 32});
		}
		
		{
			auto child = root->addChild(new Panel{});
			child->setPos({128, 5});
			child->setSize({32, 64});
		}
	}

	Context::~Context() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		delete root;
	}

	void Context::render() {
		if (!focusValid) {
			updateFocus();
			focusValid = true;
		}

		const Panel* curr = root;
		offset = {};
		//int depth = 0; // TODO: rm
		const auto focus = getFocus();

		// Breadth first traversal
		while (true) {
			while (curr) {
				if (curr->firstChild) {
					bfsNext.emplace_back(
						offset + curr->getPos(),
						curr->firstChild
					);
				}

				color.g = curr == focus ? 1.0f : 0.0f;
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
		verts.push_back({.color = color, .pos = offset + pos});
		verts.push_back({.color = color, .pos = offset + pos + glm::vec2{0, size.y}});
		verts.push_back({.color = color, .pos = offset + pos + size});

		verts.push_back({.color = color, .pos = offset + pos + size});
		verts.push_back({.color = color, .pos = offset + pos + glm::vec2{size.x, 0}});
		verts.push_back({.color = color, .pos = offset + pos});
	}

	// TODO: this is actually hover not focus...
	void Context::updateFocus() {
		Panel* const lastFocus = getFocus();
		glm::vec2 off = {};
		Panel* curr = nullptr;

		// TODO: we have a number of things to think about regarding the begin/end callbacks
		// Do we want separate onBeginChildFocus/onEndChildFocus callbacks? I think so.
		//
		// 
		// AGAIN: THIS FUNCTION SHOULD BE NAMED HOVER. MOVE THIS FOCUS STUFF ELSEWHERE.
		// We need to be able to both intercept focus and reject focus. maybe have a separate can be focused callback? looks like we already thought of that.
		// If a child rejects beginFocus it should fall back to the parent (no change if the parent was already focused, nothing should be called)
		// End focus callbacks should return void not bool. Canceling an end of focus callback doesnt really make sense.

		// Rebuild offset and find where our old focus stack loses focus
		for (auto it = focusStack.begin(), end = focusStack.end(); it != end; ++it) {
			auto* panel = *it;

			if (panel->parent == curr && (panel->getBounds() + off).contains(cursor)) {
				off += panel->getPos();
				curr = panel;
			} else {
				break;
			}
		}

		// Unwind out of focus panels
		while (!focusStack.empty()) {
			auto* back = focusStack.back();
			if (back == curr) { break; }
			// TODO: if we do want to pass through parents first we can steal the it ptr from the above for loop
			back->onEndFocus(lastFocus); // TODO: this isnt passed through parents first. May need to rethink how we handle callbacks.
			focusStack.pop_back();
		}

		// Setup the next panel to check
		curr = curr ? curr->firstChild : root;
		
		// Add any new panels to focus stack
		while (curr) {
			if ((curr->getBounds() + off).contains(cursor)) {
				off += curr->getPos();
				focusStack.push_back(curr);
				curr = curr->firstChild;
			} else {
				curr = curr->nextSibling;
			}
		}

		if (focusStack.empty()) { return; }

		Panel* const target = focusStack.back();
		if (target == lastFocus) { return; } // No change

		if (lastFocus) { lastFocus->onEndFocus(lastFocus); }
		for (Panel* panel : focusStack) {
			// TODO: do we want to cull the rest of the focus stack if we cancel early?
			// TODO: maybe it would be worth having separate onChild* callbacks. How often will we really want to intercept child callbacks?
			if (panel->onBeginFocus(target)) { break; }
		}
	}

	bool Context::onMouse(const Engine::Input::InputEvent event) {
		// ENGINE_LOG("onMouse: ", event.state.value, " @ ", Engine::Clock::Seconds{event.time.time_since_epoch()}.count());
		return false;
	}

	bool Context::onMouseMove(const Engine::Input::InputEvent event) {
		// ENGINE_LOG("onMouseMove:", " ", event.state.id.code, " ", event.state.valuef, " @ ", Engine::Clock::Seconds{event.time.time_since_epoch()}.count());

		if (event.state.id.code == 0) {
			cursor.x = event.state.valuef;
		} else {
			cursor.y = event.state.valuef;
		}
		focusValid = false;
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
