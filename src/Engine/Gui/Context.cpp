// FreeType
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>

// Engine
#include <Engine/Gui/Context.hpp>
#include <Engine/Gui/Button.hpp> // TODO: rm


namespace Engine::Gui {
	Context::Context(Engine::EngineInstance& engine) {
		polyShader = engine.shaderManager.get("shaders/gui_poly");
		glyphShader = engine.shaderManager.get("shaders/gui_glyph");
		view = engine.camera.getScreenSize(); // TODO: should update when resized

		{
			constexpr static GLuint bindingIndex = 0;

			glCreateBuffers(1, &glyphVBO);
			glyphVBOCapacity = 6 * sizeof(GlyphVertex); // TODO: just leave empty?
			glNamedBufferData(glyphVBO, glyphVBOCapacity, nullptr, GL_DYNAMIC_DRAW);

			glCreateVertexArrays(1, &glyphVAO);
			glVertexArrayVertexBuffer(glyphVAO, bindingIndex, glyphVBO, 0, sizeof(GlyphVertex));

			glEnableVertexArrayAttrib(glyphVAO, bindingIndex);
			glVertexArrayAttribBinding(glyphVAO, 0, bindingIndex);
			glVertexArrayAttribFormat(glyphVAO, 0, 2, GL_FLOAT, GL_FALSE, offsetof(GlyphVertex, pos));

			glEnableVertexArrayAttrib(glyphVAO, 1);
			glVertexArrayAttribBinding(glyphVAO, 1, bindingIndex);
			glVertexArrayAttribIFormat(glyphVAO, 1, 1, GL_UNSIGNED_INT, offsetof(GlyphVertex, index));

			glEnableVertexArrayAttrib(glyphVAO, 2);
			glVertexArrayAttribBinding(glyphVAO, 2, bindingIndex);
			glVertexArrayAttribFormat(glyphVAO, 2, 1, GL_FLOAT, GL_FALSE, offsetof(GlyphVertex, parent));
		}

		{
			quadShader = engine.shaderManager.get("shaders/fullscreen_passthrough");
			const glm::vec2 quadData[] = {
				{1,1}, {-1,1}, {-1,-1},
				{-1,-1}, {1,-1}, {1,1},
			};

			// TODO: passthrough prgoram
			glCreateBuffers(1, &quadVBO);
			glNamedBufferData(quadVBO, sizeof(quadData), &quadData, GL_STATIC_DRAW);

			glCreateVertexArrays(1, &quadVAO);
			glVertexArrayVertexBuffer(quadVAO, 0, quadVBO, 0, sizeof(quadData[0]));
			glEnableVertexArrayAttrib(quadVAO, 0);
			glVertexArrayAttribBinding(quadVAO, 0, 0);
			glVertexArrayAttribFormat(quadVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
		}

		glCreateFramebuffers(1, &fbo);

		{
			constexpr static GLuint bindingIndex = 0;

			glCreateVertexArrays(1, &polyVAO);
			glCreateBuffers(1, &polyVBO);
			glVertexArrayVertexBuffer(polyVAO, bindingIndex, polyVBO, 0, sizeof(PolyVertex));

			// Vertex
			glEnableVertexArrayAttrib(polyVAO, 0);
			glVertexArrayAttribBinding(polyVAO, 0, bindingIndex);
			glVertexArrayAttribFormat(polyVAO, 0, 4, GL_FLOAT, GL_FALSE, offsetof(PolyVertex, color));

			glEnableVertexArrayAttrib(polyVAO, 1);
			glVertexArrayAttribBinding(polyVAO, 1, bindingIndex);
			glVertexArrayAttribFormat(polyVAO, 1, 2, GL_FLOAT, GL_FALSE, offsetof(PolyVertex, pos));

			glEnableVertexArrayAttrib(polyVAO, 2);
			glVertexArrayAttribBinding(polyVAO, 2, bindingIndex);
			glVertexArrayAttribFormat(polyVAO, 2, 1, GL_FLOAT, GL_FALSE, offsetof(PolyVertex, id));
			//glVertexArrayAttribIFormat(polyVAO, 2, 1, GL_UNSIGNED_INT, offsetof(PolyVertex, id));
		
			glEnableVertexArrayAttrib(polyVAO, 3);
			glVertexArrayAttribBinding(polyVAO, 3, bindingIndex);
			glVertexArrayAttribFormat(polyVAO, 3, 1, GL_FLOAT, GL_FALSE, offsetof(PolyVertex, pid));
			//glVertexArrayAttribIFormat(polyVAO, 3, 1, GL_UNSIGNED_INT, offsetof(PolyVertex, pid));
		}

		registerPanel(nullptr); // register before everything else so nullptr = id 0

		root = new Panel{};
		root->setPos({25, 50});
		root->setSize({512, 256});
		registerPanel(root);

		///////////////////////////////////////////////////////////////////////////////

		fontId_a = fontManager.createFont("assets/arial.ttf", 32);
		fontId_b = fontManager.createFont("assets/consola.ttf", 128);
		//fontId_b = fontManager.createFont("assets/arial.ttf", 128);

		{
			auto child = new Button{};
			root->addChild(child);
			child->setPos({0, 0});
			child->setSize({64, 300});
			registerPanel(child);

			child->label = R"(Hello, world!)";
			child->label.setFont(fontId_b);

			// TODO: this is way to clunky. Find a better way
			fontManager.shapeString(child->label, fontManager.getFontGlyphSet(fontId_b));

			auto childChild = child->addChild(new Panel{});
			childChild->setPos({0, 0});
			childChild->setSize({32, 32});
			registerPanel(childChild);
		}
		
		{
			auto child = root->addChild(new Panel{});
			child->setPos({128, 5});
			child->setSize({32, 64});
			registerPanel(child);
		}

		{
			auto child = new Button{};
			root->addChild(child);
			child->setPos({256, 10});
			child->setSize({128, 64});

			child->label = R"(This is a test button)";
			child->label.setFont(fontId_a);

			// TODO: this is way to clunky. Find a better way
			//fontManager.shapeString(child->label, fontManager.getFontGlyphSet(child->label.getFont()));
			fontManager.shapeString(child->label, fontManager.getFontGlyphSet(fontId_a));

			registerPanel(child);
		}
	}

	Context::~Context() {
		glDeleteVertexArrays(1, &quadVAO);
		glDeleteBuffers(1, &quadVBO);

		glDeleteFramebuffers(1, &fbo);
		glDeleteVertexArrays(1, &polyVAO);
		glDeleteBuffers(1, &polyVBO);

		glDeleteVertexArrays(1, &glyphVAO);
		glDeleteBuffers(1, &glyphVBO);

		delete root;
	}
	

	void Context::render() {
		if (!hoverValid) {
			updateHover();
			hoverValid = true;
		}

		const Panel* curr = root;
		layer = 0;
		offset = {};
		polyDrawGroups.emplace_back().offset = 0;

		// TODO: probably move to own function
		// Breadth first traversal
		while (true) {
			// Traverse siblings
			while (curr) {
				if (curr->firstChild) {
					bfsNext.emplace_back(
						offset + curr->getPos(),
						curr->firstChild
					);
				}

				if (false) {}
				else if (curr == getActive()) { currRenderState.color = glm::vec4{1, 0, 1, 0.2}; }
				else if (curr == getHover()) { currRenderState.color = glm::vec4{1, 1, 0, 0.2}; }
				else { currRenderState.color = glm::vec4{1, 0, 0, 0.2}; }

				currRenderState.current = curr;
				currRenderState.id = getPanelId(currRenderState.current);
				currRenderState.pid = getPanelId(currRenderState.current->parent);

				const auto oldOffset = offset;
				offset += curr->getPos();
				curr->render(*this);
				offset = oldOffset;
				curr = curr->nextSibling;
			}

			// Move to next layer if needed
			if (bfsCurr.empty()) {
				const auto vsz = static_cast<GLint>(polyVertexData.size());
				polyDrawGroups.back().count = vsz - polyDrawGroups.back().offset;

				bfsCurr.swap(bfsNext);
				if (bfsCurr.empty()) { break; }

				polyDrawGroups.emplace_back().offset = vsz;
				++layer;
			}

			// Next of current layer
			const auto& back = bfsCurr.back();
			curr = back.panel;
			offset = back.offset;
			bfsCurr.pop_back();
		}

		// Build glyph vertex buffer
		if (!stringsToRender.empty()){
			std::sort(stringsToRender.begin(), stringsToRender.end(), [](const StringData& a, const StringData& b){
				return (a.layer < b.layer)
					|| (a.layer == b.layer && a.str->getFont().font < b.str->getFont().font);
			});

			// Build glyph draw groups
			int32 currLayer = stringsToRender.front().layer;
			FontId currFontId = stringsToRender.front().str->getFont();
			
			GlyphDrawGroup* group = &glyphDrawGroups.emplace_back();
			group->glyphSet = fontManager.getFontGlyphSet(currFontId);
			group->layer = currLayer;
			auto ascent = group->glyphSet->getAscent();

			for (const auto& strdat : stringsToRender) {
				if (strdat.layer != currLayer || currFontId != strdat.str->getFont()) {
					currLayer = strdat.layer;
					currFontId = strdat.str->getFont();

					GlyphDrawGroup next {
						.layer = currLayer,
						.offset = group->offset + group->count,
						.count = 0,
						.glyphSet = fontManager.getFontGlyphSet(currFontId),
					};
					group = &glyphDrawGroups.emplace_back(next);
					ascent = group->glyphSet->getAscent();
				}

				auto pos = strdat.pos;
				pos.y += ascent;
				renderString(*strdat.str, strdat.parent, pos, group->glyphSet);
				group->count += static_cast<int32>(strdat.str->getGlyphShapeData().size());
			}

			stringsToRender.clear();
		}

		{ // Update polygon vertex buffer
			// TODO: idealy we would only update if the data has actually changed
			const auto size = polyVertexData.size() * sizeof(polyVertexData[0]);
			if (size > polyVBOCapacity) {
				polyVBOCapacity = static_cast<GLsizei>(polyVertexData.capacity() * sizeof(PolyVertex));
				glNamedBufferData(polyVBO, polyVBOCapacity, nullptr, GL_DYNAMIC_DRAW);
			}
			glNamedBufferSubData(polyVBO, 0, size, polyVertexData.data());
			polyVertexData.clear();
		}
		
		{ // Update glyph vertex buffer
			const GLsizei newSize = static_cast<GLsizei>(glyphVertexData.size() * sizeof(GlyphVertex));
			if (newSize > glyphVBOCapacity) {
				ENGINE_INFO("glyphVBO(", glyphVBO, ") resize: ", newSize);
				glyphVBOCapacity = newSize;
				glNamedBufferData(glyphVBO, glyphVBOCapacity, nullptr, GL_DYNAMIC_DRAW);
			}
			glNamedBufferSubData(glyphVBO, 0, newSize, glyphVertexData.data());
			glyphVertexData.clear();
		}

		glEnable(GL_BLEND);
		glBlendFuncSeparatei(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glBlendFunci(1, GL_ONE, GL_ZERO);

		glClearTexImage(colorTex.get(), 0, GL_RGB, GL_FLOAT, 0);
		glClearTexImage(clipTex1.get(), 0, GL_RGB, GL_FLOAT, 0);
		glClearTexImage(clipTex2.get(), 0, GL_RGB, GL_FLOAT, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		// TODO: create constexpr constants for managing texture units, currently 0 is clip and 1 is glyphs

		// TODO: use UBO so we dont have to update every time we switch programs
		//// These are the same for poly and glyph shaders
		//glUniform2fv(0, 1, &view.x); 
		//glUniform1i(1, 0);
		//
		//// Only used by glyph shader
		//glUniform1i(2, 1);

		// We cant use glMultiDrawArrays because you can not read/write
		// the same texture. It may be possible to work around this
		// with glTextureBarrier but that isnt as widely supported.

		auto currGlyphDrawGroup = glyphDrawGroups.data();
		const auto lastGlyphDrawGroup = currGlyphDrawGroup + glyphDrawGroups.size();
		GLuint activeStage = 0;

		auto swapClipBuffers = [&]() ENGINE_INLINE {
			activeClipTex = !activeClipTex;
			const GLenum buffs[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 + activeClipTex};
			glNamedFramebufferDrawBuffers(fbo, 2, &buffs[0]);
			glBindTextureUnit(0, activeClipTex ? clipTex1.get() : clipTex2.get());
		};

		// Setup clipping buffers
		swapClipBuffers();

		for (int32 layer = 0; layer < polyDrawGroups.size(); ++layer) {
			const auto first = polyDrawGroups[layer].offset;
			const auto count = polyDrawGroups[layer].count;

			// Draw polys
			{
				// TODO: will need to swap program/vao/uniforms
				if (activeStage != polyVAO) {
					activeStage = polyVAO;
					glBindVertexArray(polyVAO);
					glUseProgram(polyShader->get());
					
					// TODO: use UBO so we dont have to update every time we switch programs
					glUniform2fv(0, 1, &view.x); 
					glUniform1i(1, 0);
					//glUniform1i(2, 1);
				}

				glDrawArrays(GL_TRIANGLES, first, count);
			}

			swapClipBuffers();

			// Draw glyphs
			{
				if (currGlyphDrawGroup == lastGlyphDrawGroup) { continue; }
				if (currGlyphDrawGroup->layer != layer) { continue; }

				// Disable drawing to clip buffer
				glBlendFunci(1, GL_ZERO, GL_ONE);

				if (activeStage != glyphVAO) {
					activeStage = glyphVAO;
					glBindVertexArray(glyphVAO);
					glUseProgram(glyphShader->get());
					
					// TODO: use UBO so we dont have to update every time we switch programs
					glUniform2fv(0, 1, &view.x); 
					glUniform1i(1, 0);
					glUniform1i(2, 1);
				}

				FontGlyphSet* activeSet = nullptr;

				while (true) {
					if (currGlyphDrawGroup->glyphSet != activeSet) {
						activeSet = currGlyphDrawGroup->glyphSet;

						// TODO: this needs once per set not per draw (layer + group):
						activeSet->updateDataBuffer();

						glBindTextureUnit(1, activeSet->getGlyphTexture().get());
						glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, activeSet->getGlyphDataBuffer());
					}

					//ENGINE_LOG("Draw Glyphs: layer(", currGlyphDrawGroup->layer, "), offset(", currGlyphDrawGroup->offset, ") count(", currGlyphDrawGroup->count, ")");
					glDrawArrays(GL_POINTS, currGlyphDrawGroup->offset, currGlyphDrawGroup->count);

					++currGlyphDrawGroup;
					if (currGlyphDrawGroup == lastGlyphDrawGroup) { break; }
					if (currGlyphDrawGroup->layer != layer) { break; }
				}

				// Enable drawing to clip buffer
				glBlendFunci(1, GL_ONE, GL_ZERO);
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Draw to main framebuffer
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(quadVAO);
		glUseProgram(quadShader->get());
		glBindTextureUnit(0, colorTex.get());
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Reset buffers
		glDisable(GL_BLEND);
		polyDrawGroups.clear();
		glyphDrawGroups.clear();

		constexpr const char* lines[] = {
			R"(Alice was beginning to get very tired of sitting by her sister on the bank, and of having nothing to do: once or twice she)",
			R"(had peeped into the book her sister was reading, but it had no pictures or conversations in it, "and what is the use of a)",
			R"(book," thought Alice, "without pictures or conversations?" So she was considering in her own mind, (as well as she could,)",
			R"(for the hot day made her feel very sleepy and stupid,) whether the pleasure of making a daisy-chain would be worth the trouble)",
			R"(of getting up and picking the daisies, when suddenly a white rabbit with pink eyes ran close by her. There was nothing so)",
			R"(very remarkable in that; nor did Alice think it so very much out of the way to hear the Rabbit say to itself, "Oh dear! Oh)",
			R"(dear! I shall be too late!" (when she thought it over afterwards, it occurred to her that she ought to have wondered at this,)",
			R"(but at the time it all seemed quite natural;) but when the Rabbit actually took a watch out of its waistcoat-pocket, and)",
			R"(looked at it, and then hurried on, Alice started to her feet, for it flashed across her mind that she had never before seen)",
			R"(a rabbit with either a waistcoat-pocket, or a watch to take out of it, and, burning with curiosity, she ran across the field)",
			R"(after it, and was just in time to see it pop down a large rabbit-hole under the hedge. In another moment down went Alice)",
			R"(after it, never once considering how in the world she was to get out again. The rabbit-hole went straight on like a tunnel)",
			R"(for some way, and then dipped suddenly down, so suddenly that Alice had not a moment to think about stopping herself before)",
			R"(she found herself falling down what seemed to be a very deep well. Either the well was very deep, or she fell very slowly,)",
			R"(for she had plenty of time as she went down to look about her, and to wonder what was going to happen next. First, she tried)",
			R"(to look down and make out what she was coming to, but it was too dark to see anything: then she looked at the sides of the)",
			R"(well, and noticed that they were filled with cupboards and book-shelves: here and there she saw maps and pictures hung upon)",
			R"(pegs. She took down a jar from one of the shelves as she passed; it was labelled "ORANGE MARMALADE," but to her great disappointment)",
			R"----(T̴̖̀è̴̖s̴̖̀t̴̖̀)----",
			R"----(😀👍)----",
			//R"(it was empty: she did not like to drop the jar for fear of killing somebody underneath, so managed to put it into one of)",
			//R"(the cupboards as she fell past it. "Well!" thought Alice to herself, "after such a fall as this, I shall think nothing of)",
			//R"(tumbling down stairs! How brave they'll all think me at home! Why, I wouldn't say anything about it, even if I fell off the)",
			//R"(top of the house!" (Which was very likely true.) Down, down, down. Would the fall never come to an end! "I wonder how many)",
			//R"(miles I've fallen by this time'?" she said aloud. "I must be getting somewhere near the centre of the earth. Let me see:)",
			//R"(that would be four thousand miles down, I think—" (for, you see, Alice had learnt several things of this sort in her lessons)",
			//R"(in the schoolroom, and though this was not a very good opportunity for showing off her knowledge, as there was no one to)",
			//R"(listen to her, still it was good practice to say it over) "—yes, that's about the right distance—but then I wonder what Latitude)",
			//R"(or Longitude I've got to?" (Alice had not the slightest idea what Latitude was, or Longitude either, but she thought they)",
			//R"(were nice grand words to say.) Presently she began again. "I wonder if I shall fall right through the earth! How funny it'll)",
			//R"(seem to come out among the people that walk with their heads downwards! The Antipathies, I think—" (she was rather glad there)",
			//R"(was no one listening, this time, as it didn't sound at all the right word) "—but I shall have to ask them what the name of)",
			//R"(the country is, you know. Please, Ma'am, is this New Zealand or Australia?" (and she tried to curtsey as she spoke—fancy)",
			//R"(curtseying as you're falling through the air! Do you think you could manage it?) "And what an ignorant little girl she'll)",
			//R"(think me for asking! No, it'll never do to ask: perhaps I shall see it written up somewhere." Down, down, down. There was)",
			//R"(nothing else to do, so Alice soon began talking again. "Dinah'll miss me very much to-night, I should think!" (Dinah was)",
			//R"(the cat.) "I hope they'll remember her saucer of milk at tea-time. Dinah, my dear! I wish you were down here with me! There)",
			//R"(are no mice in the air, I'm afraid, but you might catch a bat, and that's very like a mouse, you know. But do cats eat bats,)",
		};

		static ShapedString fontLines_a[std::size(lines)];
		static ShapedString fontLines_b[std::size(lines)];
		static int initShapedLines = [&](){
			//ENGINE_LOG("initShapedLines");
			//auto* glyphSet_a = fontManager.getFontGlyphSet(fontId_a);
			//for (int i = 0; i < std::size(fontLines_a); ++i) {
			//	fontLines_a[i] = lines[i];
			//	fontLines_a[i].setFont(fontId_a);
			//	fontManager.shapeString(fontLines_a[i], glyphSet_a);
			//}
			//
			//auto* glyphSet_b = fontManager.getFontGlyphSet(fontId_b);
			//for (int i = 0; i < std::size(fontLines_b); ++i) {
			//	fontLines_b[i] = lines[i];
			//	fontLines_b[i].setFont(fontId_b);
			//	fontManager.shapeString(fontLines_b[i], glyphSet_b);
			//}
			return 0;
		}();

		static Clock::duration avg = {};
		static int avgCounter = {};
		const auto startT = Clock::now();

		/*{
			float32 line = 0;

			const auto line_a = fontManager.getFontGlyphSet(fontId_a)->getLineHeight();
			const auto line_b = fontManager.getFontGlyphSet(fontId_b)->getLineHeight();

			for (const auto& text : fontLines_a) {
				drawString({10, line += line_a}, &text);
			}
			for (const auto& text : fontLines_b) {
				drawString({10, line += line_b}, &text);
			}
		}*/

		const auto endT = Clock::now();
		const auto diff = endT - startT;
		avg += diff;
		if (++avgCounter == 100) {
			avg /= avgCounter;
			//ENGINE_LOG("Glyph time: ", Clock::Milliseconds{avg}.count(), "ms");
			avgCounter = 0;
		}
	}

	void Context::drawRect(const glm::vec2 pos, const glm::vec2 size) {
		const PanelId id = currRenderState.id;
		const PanelId pid = currRenderState.pid;
		const auto color = currRenderState.color;

		polyVertexData.push_back({.color = color, .pos = offset + pos, .id = id, .pid = pid});
		polyVertexData.push_back({.color = color, .pos = offset + pos + glm::vec2{0, size.y}, .id = id, .pid = pid});
		polyVertexData.push_back({.color = color, .pos = offset + pos + size, .id = id, .pid = pid});

		polyVertexData.push_back({.color = color, .pos = offset + pos + size, .id = id, .pid = pid});
		polyVertexData.push_back({.color = color, .pos = offset + pos + glm::vec2{size.x, 0}, .id = id, .pid = pid});
		polyVertexData.push_back({.color = color, .pos = offset + pos, .id = id, .pid = pid});
	}

	void Context::drawString(glm::vec2 pos, const ShapedString* fstr) {
		stringsToRender.emplace_back(layer, currRenderState.id, pos + offset, fstr);
	}

	void Context::updateHover() {
		Panel* const old = getHover();
		glm::vec2 off = {};
		Panel* curr = nullptr;

		// Rebuild offset and find where our old stack ends
		{
			auto it = hoverStack.begin();
			auto end = hoverStack.end();

			for (; it != end; ++it) {
				auto* panel = *it;

				if (panel->canHover() && panel->parent == curr && (panel->getBounds() + off).contains(cursor)) {
					off += panel->getPos();
					curr = panel;
				} else {
					break;
				}
			}

			if (it != end) {
				const auto start = it;
				auto next = it + 1;
				while (next != end) {
					(*it)->onEndChildHover(*next);
					it = next;
					++next;
				}

				hoverStack.erase(start, end);
			}
		}

		// Setup the next panel to check
		curr = curr ? curr->firstChild : root;

		while (curr) {
			if (curr->canHover() && (curr->getBounds() + off).contains(cursor)) {
				if (curr->parent && curr->parent->onBeginChildHover(curr)) {
					break;
				}

				off += curr->getPos();
				hoverStack.push_back(curr);
				curr = curr->firstChild;
			} else {
				curr = curr->nextSibling;
			}
		}

		Panel* target = getHover();
		if (target != old) {
			if (old) { old->onEndHover(); };
			if (target) { target->onBeginHover(); }
		}
	}

	void Context::renderString(const ShapedString& str, PanelId parent, glm::vec2 base, FontGlyphSet* font) {
		const auto glyphShapeData = str.getGlyphShapeData();

		for (const auto& data : glyphShapeData) {
			const uint32 index = font->getGlyphIndex(data.index);
			glyphVertexData.push_back({
				.pos = glm::round(base + data.offset),
				.index = index,
				.parent = parent,
			});
			base += data.advance;
		}
	}

	bool Context::onMouse(const Engine::Input::InputEvent event) {
		//ENGINE_LOG("onMouse:",
		//	" ", event.state.value,
		//	" ", (int)event.state.id.code,
		//	" ", (int)event.state.id.type,
		//	" ", (int)event.state.id.device,
		//	" @ ", Engine::Clock::Seconds{event.time.time_since_epoch()}.count()
		//);
		if (event.state.id.code == 0) {
			if (event.state.value && isHoverAny()) {
				ENGINE_DEBUG_ASSERT(active == nullptr);
				auto focus = getFocus();
				if (!focus) { return false; }
				focus->onBeginActivate();
				active = focus;
				return true;
			} else if (active) {
				active->onEndActivate();
				active = nullptr;
				return true;
			}
		}
		return false;
	}

	bool Context::onMouseMove(const Engine::Input::InputEvent event) {
		//ENGINE_LOG("onMouseMove:", " ", event.state.id.code, " ", event.state.valuef, " @ ", Engine::Clock::Seconds{event.time.time_since_epoch()}.count());
		if (event.state.id.code == 0) {
			cursor.x = event.state.valuef;
		} else {
			cursor.y = event.state.valuef;
		}
		hoverValid = false;
		return isHoverAny();
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
		if (w == view.x && h == view.y) { return; }
		ENGINE_LOG("onResize: ", w, " ", h);
		view = {w, h};

		// TODO: can we use a u16 for clip textures?
		colorTex.setStorage(TextureFormat::RGBA8, view);
		clipTex1.setStorage(TextureFormat::R32F, view);
		clipTex2.setStorage(TextureFormat::R32F, view);

		glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, colorTex.get(), 0);
		glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT1, clipTex1.get(), 0);
		glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT2, clipTex2.get(), 0);
	}

	void Context::onFocus(const bool has) {
		//ENGINE_LOG("onFocus: ", has);
		if (!has) {
			hoverValid = true;

			// Remove any hovers
			if (!hoverStack.empty()) {
				auto it = hoverStack.begin();
				auto end = hoverStack.end();
				auto next = it + 1;

				while (next != end) {
					(*it)->onEndChildHover(*next);
					it = next;
					++next;
				}

				(*it)->onEndHover();
				hoverStack.clear();
			}
		} else {
			hoverValid = false;
		}
	}

}
