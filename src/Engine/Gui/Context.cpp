// FreeType
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>

// Engine
#include <Engine/Gui/Context.hpp>

// TODO: rm
#include <imgui.h>


namespace Engine::Gui {
	Context::Context(Engine::EngineInstance& engine) {
		shader = engine.shaderManager.get("shaders/gui_clip");
		glyphShader = engine.shaderManager.get("shaders/gui_text2");
		view = engine.camera.getScreenSize(); // TODO: should update when resized

		{
			constexpr float32 mscale = 1.0f/64;
			FT_Library ftlib;
			if (const auto err = FT_Init_FreeType(&ftlib)) {
				ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
			}

			FT_Face face;
			if (const auto err = FT_New_Face(ftlib, "assets/arial.ttf", 0, &face)) {
				ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
			}

			if (const auto err = FT_Set_Pixel_Sizes(face, 0, 32)) {
				ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
			}

			ENGINE_LOG("FaceX: ", face->bbox.xMax - face->bbox.xMin);
			ENGINE_LOG("FaceY: ", face->bbox.yMax - face->bbox.yMin);
			ENGINE_LOG("FaceXPx: ", FT_MulFix(face->bbox.xMax - face->bbox.xMin, face->size->metrics.x_scale) * (1.0/64));
			ENGINE_LOG("FaceYPx: ", FT_MulFix(face->bbox.yMax - face->bbox.yMin, face->size->metrics.y_scale) * (1.0/64));
			ENGINE_LOG("UPM: ", face->units_per_EM);

			// In practice most glyphs are much smaller than `face->bbox` (every ascii char is < 1/2 the size depending on font)
			// It would be better to use a packing algorithm, but then we would want
			// to periodically repack (glCopy*, not reload) as glyphs are unloaded which complicates things.
			// Still very doable. Just requires more work.
			GLint texSize;
			glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
			ENGINE_INFO("Maximum texture size: ", texSize);

			// TODO: may want to limit base on face size? for small font sizes 4096 might be excessive.
			texSize = std::min(texSize, 4096);

			// TODO: i think this may be subtly incorrect.
			// 
			// I think this is the bounding box that could fit ALL glyphs at the same
			// time. Not a box that is large enough to contain any given glyph.
			// 
			// For example:
			// We have one glyph whos outline is bound by [0, 10] (10u wide).
			// We have another glyph whos outline is not positioned at the origin
			// so it is bound by [80, 100] (20u wide).
			// The bbox needed to contain these is only 20u wide, but the bbox
			// needed to contain BOTH is [0, 100] (100u wide).
			// I think this is what face->bbox is?
			// 
			// After some testing the above does appear to be correct.
			// To calc the bbox that we need (maximum size of any single glyph) takes
			// 250ms/150ms (debug/release) per 4000 glyphs with a 5820k @ 4.1GHz.
			// The max number of glyphs a font can have is 65,535 which would take ~4.1s
			//
			// If we calc the bbox in font units we should be able to only do
			// it once per font and then scale by pixel size (face.size.metrics.x/y_scale?)
			//
			// Although the size difference doesnt seem to be that large. Maybe its not worth doing?
			//
			maxFace = glm::ceil(glm::vec2{
				FT_MulFix(face->bbox.xMax - face->bbox.xMin, face->size->metrics.x_scale) * mscale,
				FT_MulFix(face->bbox.yMax - face->bbox.yMin, face->size->metrics.y_scale) * mscale
			});

			//glm::vec2 maxGlyph = {};
			//const auto startT = Clock::now();
			//for (int i = 0; FT_Load_Glyph(face, i, FT_LOAD_DEFAULT) == 0; ++i) {
			//	FT_Glyph glyph;
			//	FT_BBox bbox;
			//	if (FT_Get_Glyph(face->glyph, &glyph)) { ENGINE_WARN("Oh no!"); break; }
			//	FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_GRIDFIT, &bbox);
			//	FT_Done_Glyph(glyph);
			//	maxGlyph.x = std::max(maxGlyph.x, (bbox.xMax - bbox.xMin) * mscale);
			//	maxGlyph.y = std::max(maxGlyph.y, (bbox.yMax - bbox.yMin) * mscale);
			//}
			//const auto endT = Clock::now();
			//ENGINE_LOG("Time: ", Clock::Milliseconds{endT-startT}.count());
			//ENGINE_LOG("Max Glyph: ", maxGlyph.x, ", ", maxGlyph.y);
			//ENGINE_LOG("Max Face: ", maxFace.x, ", ", maxFace.y);

			indexBounds = glm::floor(glm::vec2{texSize, texSize} / maxFace);
				
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			glyphTex.setStorage(TextureFormat::R8, {texSize, texSize});
			for (uint8 c = ' '; c <= '~'; ++c) {
				if (const auto err = FT_Load_Char(face, c, FT_LOAD_RENDER)) {
					ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
				}
				const auto& glyph = *face->glyph;
				const auto& metrics = glyph.metrics;

				auto& met = glyphMetrics.emplace_back();
				met.index = nextGlyphIndex;
				met.bearing = {metrics.horiBearingX * mscale, metrics.horiBearingY * -mscale};
				met.advance = metrics.horiAdvance * mscale;

				auto& dat = glyphData.emplace_back();
				dat.size = {metrics.width * mscale, metrics.height * mscale};
				charToIndex[c] = nextGlyphIndex;
				++nextGlyphIndex;

				if (glyph.bitmap.width) {
					const glm::vec2 index = {
						met.index % indexBounds.x,
						met.index / indexBounds.x,
					};
					dat.offset = glm::vec3{index * maxFace, 0 };

					ENGINE_DEBUG_ASSERT(index.x <= indexBounds.x);
					ENGINE_DEBUG_ASSERT(index.y <= indexBounds.y);

					//ENGINE_LOG("C: ", c, " - ", index.x, ",", index.y, " - ", offset.x, ",", offset.y);

					glyphTex.setSubImage(0, dat.offset, dat.size, PixelFormat::R8, glyph.bitmap.buffer);
				}
			}

			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

			if (const auto err = FT_Done_Face(face)) {
				ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
			}
			
			if (const auto err = FT_Done_FreeType(ftlib)) {
				ENGINE_ERROR("FreeType error: ", err); // TODO: actual error
			}
			
			glCreateBuffers(1, &glyphSSBO);
			glCreateBuffers(1, &glyphVBO);
			glyphVBOSize = 6 * sizeof(GlyphVertex); // TODO: just leave empty?
			glNamedBufferData(glyphVBO, glyphVBOSize, nullptr, GL_DYNAMIC_DRAW);

			glCreateVertexArrays(1, &glyphVAO);
			glVertexArrayVertexBuffer(glyphVAO, 0, glyphVBO, 0, sizeof(GlyphVertex));

			glEnableVertexArrayAttrib(glyphVAO, 0);
			glVertexArrayAttribBinding(glyphVAO, 0, 0);
			glVertexArrayAttribFormat(glyphVAO, 0, 2, GL_FLOAT, GL_FALSE, offsetof(GlyphVertex, pos));

			glEnableVertexArrayAttrib(glyphVAO, 1);
			glVertexArrayAttribBinding(glyphVAO, 1, 0);
			glVertexArrayAttribIFormat(glyphVAO, 1, 2, GL_UNSIGNED_INT, offsetof(GlyphVertex, index));

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

		glCreateVertexArrays(1, &vao);
		glCreateBuffers(1, &vbo);
		glVertexArrayVertexBuffer(vao, vertBindingIndex, vbo, 0, sizeof(Vertex));

		// Vertex
		glEnableVertexArrayAttrib(vao, 0);
		glVertexArrayAttribBinding(vao, 0, vertBindingIndex);
		glVertexArrayAttribFormat(vao, 0, 4, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));

		glEnableVertexArrayAttrib(vao, 1);
		glVertexArrayAttribBinding(vao, 1, vertBindingIndex);
		glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));

		glEnableVertexArrayAttrib(vao, 2);
		glVertexArrayAttribBinding(vao, 2, vertBindingIndex);
		glVertexArrayAttribFormat(vao, 2, 1, GL_FLOAT, GL_FALSE, offsetof(Vertex, id));
		//glVertexArrayAttribIFormat(vao, 2, 1, GL_UNSIGNED_INT, offsetof(Vertex, id));
		
		glEnableVertexArrayAttrib(vao, 3);
		glVertexArrayAttribBinding(vao, 3, vertBindingIndex);
		glVertexArrayAttribFormat(vao, 3, 1, GL_FLOAT, GL_FALSE, offsetof(Vertex, pid));
		//glVertexArrayAttribIFormat(vao, 3, 1, GL_UNSIGNED_INT, offsetof(Vertex, pid));

		registerPanel(nullptr); // register before everything else so nullptr = id 0

		root = new Panel{};
		root->setPos({25, 50});
		root->setSize({512, 256});
		registerPanel(root);

		{
			auto child = root->addChild(new Panel{});
			child->setPos({0, 0});
			child->setSize({64, 300});
			registerPanel(child);

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
	}

	Context::~Context() {
		glDeleteVertexArrays(1, &quadVAO);
		glDeleteBuffers(1, &quadVBO);

		glDeleteFramebuffers(1, &fbo);
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);

		glDeleteVertexArrays(1, &glyphVAO);
		glDeleteBuffers(1, &glyphVBO);
		glDeleteBuffers(1, &glyphSSBO);

		delete root;
	}
	
	void Context::renderText2(const std::string_view str, glm::vec2 base) {
		for (const uint8 c : str) {
			const auto i = charToIndex[c];
			//const auto& dat = glyphData[i];
			const auto& met = glyphMetrics[i];

			// TODO: dont draw whitespace
			auto p = base + met.bearing;

			// TODO: rm
			// TODO: probably just store offset or uvs in glyph met
			//const glm::vec2 index = { 
			//	dat.index % indexBounds.x,
			//	dat.index / indexBounds.x,
			//};
			//const glm::vec2 offset = index * maxFace;
			//
			//glm::vec2 uvBegin = offset / 4096.0f;
			//glm::vec2 uvEnd = uvBegin + (dat.size / 4096.0f);
			//
			//glyphVertexData.push_back({{p.x, p.y}, {uvBegin.x, uvBegin.y}});
			//glyphVertexData.push_back({{p.x, p.y + dat.size.y}, {uvBegin.x, uvEnd.y}});
			//glyphVertexData.push_back({{p.x + dat.size.x, p.y}, {uvEnd.x, uvBegin.y}});
			//glyphVertexData.push_back({{p.x, p.y + dat.size.y}, {uvBegin.x, uvEnd.y}});
			//glyphVertexData.push_back({{p.x + dat.size.x, p.y + dat.size.y}, {uvEnd.x, uvEnd.y}});
			//glyphVertexData.push_back({{p.x + dat.size.x, p.y}, {uvEnd.x, uvBegin.y}});
			glyphVertexData.push_back({{p.x, p.y}, met.index});

			// Assume we want a horizontal layout
			base.x += met.advance;
		}
	}

	void Context::render() {
		if (!hoverValid) {
			updateHover();
			hoverValid = true;
		}


		// TODO: rm - for debugging NSight
		if (ImGui::Begin("Test")) {
			ImGui::Text("Hello!");
			ImTextureID tid = reinterpret_cast<void*>(static_cast<uintptr_t>(glyphTex.get()));
			//ImGui::Image(tid, ImVec2(static_cast<float32>(512), static_cast<float32>(512)), {0,1}, {1,0});
			ImGui::Image(tid, ImVec2(static_cast<float32>(4096), static_cast<float32>(4096)));
		} ImGui::End();

		const Panel* curr = root;
		offset = {};
		multiDrawData.first.emplace_back() = 0;
		
		// Breadth first traversal
		while (true) {
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
				curr->render(*this);
				curr = curr->nextSibling;
			}

			if (bfsCurr.empty()) {
				const auto vsz = static_cast<GLint>(verts.size());
				multiDrawData.count.emplace_back() = vsz - multiDrawData.first.back();

				bfsCurr.swap(bfsNext);
				if (bfsCurr.empty()) { break; }

				multiDrawData.first.emplace_back() = vsz;
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
		glUseProgram(shader->get());
		glUniform2fv(0, 1, &view.x);

		glEnable(GL_BLEND);
		glBlendFuncSeparatei(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glBlendFunci(1, GL_ONE, GL_ZERO);

		// If we dont care about clipping we can just do
		//glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(verts.size()));

		glClearTexImage(colorTex.get(), 0, GL_RGB, GL_FLOAT, 0);
		glClearTexImage(clipTex1.get(), 0, GL_RGB, GL_FLOAT, 0);
		glClearTexImage(clipTex2.get(), 0, GL_RGB, GL_FLOAT, 0);

		// We cant use glMultiDrawArrays because you can not read/write
		// the same texture. It may be possible to work around this
		// with glTextureBarrier but that isnt as widely supported.
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		for (int i = 0; i < multiDrawData.first.size(); ++i) {
			const auto first = multiDrawData.first[i];
			const auto count = multiDrawData.count[i];

			const GLenum buffs[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 + activeClipTex};
			glNamedFramebufferDrawBuffers(fbo, 2, &buffs[0]);
			glBindTextureUnit(0, activeClipTex ? clipTex1.get() : clipTex2.get());
			glDrawArrays(GL_TRIANGLES, first, count);
			activeClipTex = !activeClipTex;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(quadVAO);
		glUseProgram(quadShader->get());
		glBindTextureUnit(0, colorTex.get());
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisable(GL_BLEND);
		verts.clear();
		multiDrawData.first.clear();
		multiDrawData.count.clear();

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
			R"(it was empty: she did not like to drop the jar for fear of killing somebody underneath, so managed to put it into one of)",
			R"(the cupboards as she fell past it. "Well!" thought Alice to herself, "after such a fall as this, I shall think nothing of)",
			R"(tumbling down stairs! How brave they'll all think me at home! Why, I wouldn't say anything about it, even if I fell off the)",
			R"(top of the house!" (Which was very likely true.) Down, down, down. Would the fall never come to an end! "I wonder how many)",
			R"(miles I've fallen by this time'?" she said aloud. "I must be getting somewhere near the centre of the earth. Let me see:)",
			R"(that would be four thousand miles down, I think—" (for, you see, Alice had learnt several things of this sort in her lessons)",
			R"(in the schoolroom, and though this was not a very good opportunity for showing off her knowledge, as there was no one to)",
			R"(listen to her, still it was good practice to say it over) "—yes, that's about the right distance—but then I wonder what Latitude)",
			R"(or Longitude I've got to?" (Alice had not the slightest idea what Latitude was, or Longitude either, but she thought they)",
			R"(were nice grand words to say.) Presently she began again. "I wonder if I shall fall right through the earth! How funny it'll)",
			R"(seem to come out among the people that walk with their heads downwards! The Antipathies, I think—" (she was rather glad there)",
			R"(was no one listening, this time, as it didn't sound at all the right word) "—but I shall have to ask them what the name of)",
			R"(the country is, you know. Please, Ma'am, is this New Zealand or Australia?" (and she tried to curtsey as she spoke—fancy)",
			R"(curtseying as you're falling through the air! Do you think you could manage it?) "And what an ignorant little girl she'll)",
			R"(think me for asking! No, it'll never do to ask: perhaps I shall see it written up somewhere." Down, down, down. There was)",
			R"(nothing else to do, so Alice soon began talking again. "Dinah'll miss me very much to-night, I should think!" (Dinah was)",
			R"(the cat.) "I hope they'll remember her saucer of milk at tea-time. Dinah, my dear! I wish you were down here with me! There)",
			R"(are no mice in the air, I'm afraid, but you might catch a bat, and that's very like a mouse, you know. But do cats eat bats,)",
/*			R"(I wonder?" And here Alice began to get rather sleepy, and went on saying to herself, in a dreamy sort of way, "Do cats eat)",
			R"(bats? Do cats eat bats?" and sometimes, "Do bats eat cats?" for, you see, as she couldn't answer either question, it didn't)",
			R"(much matter which way she put it. She felt that she was dozing off, and had just begun to dream that she was walking hand)",
			R"(in hand with Dinah, and was saying to her very earnestly, "Now, Dinah, tell me the truth: did you ever eat a bat?" when suddenly,)",
			R"(thump! thump! down she came upon a heap of sticks and dry leaves, and the fall was over. Alice was not a bit hurt, and she)",
			R"(jumped up on to her feet in a moment: she looked up, but it was all dark overhead; before her was another long passage, and)",
			R"(the White Rabbit was still in sight, hurrying down it. There was not a moment to be lost: away went Alice like the wind,)",
			R"(and was just in time to hear it say, as it turned a corner, "Oh my ears and whiskers, how late it's getting!" She was close)",
			R"(behind it when she turned the corner, but the Rabbit was no longer to be seen: she found herself in a long, low hall, which)",
			R"(was lit up by a row of lamps hanging from the roof. There were doors all round the hall, but they were all locked; and when)",
			R"(Alice had been all the way down one side and up the other, trying every door, she walked sadly down the middle, wondering)",
			R"(how she was ever to get out again. Suddenly she came upon a little three-legged table, all made of solid glass; there was)",
			R"(nothing on it but a tiny golden key, and Alice's first idea was that this might belong to one of the doors of the hall; but,)",
			R"(alas! either the locks were too large, or the key was too small, but at any rate it would not open any of them. However,)",
			R"(on the second time round, she came upon a low curtain she had not noticed before, and behind it was a little door about fifteen)",
			R"(inches high: she tried the little golden key in the lock, and to her great delight it fitted! Alice opened the door and found)",
			R"(that it led into a small passage, not much larger than a rat-hole: she knelt down and looked along the passage into the loveliest)",
		*/};

		static Clock::duration avg = {};
		static int avgCounter = {};
		const auto startT = Clock::now();
		for (int n = 0; auto text : lines) {
			//renderText2(text, {10, 32 * ++n});
			n;text;
		}
		renderText2("Hello, world!", {32, 512});
		{
			glBindVertexArray(glyphVAO);
			glUseProgram(glyphShader->get());
			glBindTextureUnit(0, glyphTex.get());
			glUniform2fv(0, 1, &view.x);
			glUniform1i(1, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, glyphSSBO);

			{
				const GLsizei newSize = static_cast<GLsizei>(glyphVertexData.size() * sizeof(GlyphVertex));
				if (newSize > glyphVBOSize) {
					ENGINE_INFO("glyphVBO resize: ", newSize);
					glyphVBOSize = newSize;
					glNamedBufferData(glyphVBO, glyphVBOSize, nullptr, GL_DYNAMIC_DRAW);
				}
				glNamedBufferSubData(glyphVBO, 0, newSize, glyphVertexData.data());
			}

			{ // TODO: we should know when this is resized. just do this then?
				const GLsizei newSize = static_cast<GLsizei>(glyphData.size() * sizeof(glyphData[0]));
				if (newSize > glyphSSBOSize) {
					ENGINE_INFO("glyphSSBO resize: ", newSize, " ", glyphSSBO);
					glyphSSBOSize = newSize;
					glNamedBufferData(glyphSSBO, glyphSSBOSize, nullptr, GL_DYNAMIC_DRAW); // TODO: what storge type?
					glNamedBufferSubData(glyphSSBO, 0, newSize, glyphData.data());
				}
			}

			//glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(glyphVertexData.size()));
			glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(glyphVertexData.size()));
			glyphVertexData.clear();
		}
		const auto endT = Clock::now();
		const auto diff = endT - startT;
		avg += diff;
		if (++avgCounter == 100) {
			avg /= avgCounter;
			ENGINE_LOG("Glyph time: ", Clock::Milliseconds{avg}.count(), "ms");
			avgCounter = 0;
		}
	}

	void Context::addRect(const glm::vec2 pos, const glm::vec2 size) {
		const PanelId id = getPanelId(currRenderState.current);
		const PanelId pid = getPanelId(currRenderState.current->parent);
		const auto color = currRenderState.color;

		verts.push_back({.color = color, .pos = offset + pos, .id = id, .pid = pid});
		verts.push_back({.color = color, .pos = offset + pos + glm::vec2{0, size.y}, .id = id, .pid = pid});
		verts.push_back({.color = color, .pos = offset + pos + size, .id = id, .pid = pid});

		verts.push_back({.color = color, .pos = offset + pos + size, .id = id, .pid = pid});
		verts.push_back({.color = color, .pos = offset + pos + glm::vec2{size.x, 0}, .id = id, .pid = pid});
		verts.push_back({.color = color, .pos = offset + pos, .id = id, .pid = pid});
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
