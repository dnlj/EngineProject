// Windows
#include <Windows.h>
#include <io.h>

// STD
#include <iostream>
#include <numeric>
#include <filesystem>
#include <csignal>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/TextureManager.hpp>
#include <Engine/EngineInstance.hpp>
#include <Engine/ResourceManager.hpp>
#include <Engine/Noise/OpenSimplexNoise.hpp>
#include <Engine/Noise/SimplexNoise.hpp>
#include <Engine/Noise/WorleyNoise.hpp>
#include <Engine/Win32/Win32.hpp>
#include <Engine/WindowCallbacks.hpp>
#include <Engine/Window.hpp>
#include <Engine/ImGui/ImGui.hpp>
#include <Engine/Input/InputSequence.hpp>
#include <Engine/CommandLine/Parser.hpp>
#include <Engine/Debug/GL/GL.hpp>
#include <Engine/ConfigParser.hpp>
#include <Engine/Gui/Context.hpp>

// Game
#include <Game/Common.hpp>
#include <Game/World.hpp>


namespace {
	using namespace Engine::Types;
	constexpr int32 OPENGL_VERSION_MAJOR = 4;
	constexpr int32 OPENGL_VERSION_MINOR = 5;
	double avgDeltaTime = 0.0;

	static Engine::Clock::TimePoint launchTime;
	static Engine::Clock::TimePoint startTime;

	struct {
		constexpr static int w = 512;
		constexpr static int h = 512;
		GLuint tex = 0;
		//GLuint tex2 = 0;
		//float32 data2[w] = {};
		//float32 data[w] = {};
	} map;
	
	void mapTest(float32 xOffset = 0, float32 yOffset = 0, float32 xZoom = 1.0f, float32 yZoom = 1.0f) {
		struct Color {
			uint8_t r = 0;
			uint8_t g = 0;
			uint8_t b = 0;

			Color() = default;
			Color(int r, int g, int b) : r{static_cast<uint8_t>(r)}, g{static_cast<uint8_t>(g)}, b{static_cast<uint8_t>(b)} {}
			explicit Color(uint32_t value) { *this = value; }

			Color& operator=(uint32_t value) {
				r = (value & 0x00FF0000) >> 16;
				g = (value & 0x0000FF00) >> 8;
				b = (value & 0x000000FF) >> 0;
				return *this;
			}

			void gray(uint8_t value) {
				r = g = b = value;
			}
		};

		Color data[map.h][map.w];
		//Color data2[map.h][map.w];
		
		/*ENGINE_INLINE_CALLS {
			std::ios_base::sync_with_stdio(false);
			Engine::Noise::SimplexNoise simplex{1234};
			//Engine::Noise::OpenSimplexNoise simplex{1234};
			constexpr float32 r = 100000;
			float32 min = FLT_MAX;
			float32 max = FLT_MIN;
			uint8 p = 0;

			float32 scales[] = {
				11.168912f,
				11.1234f,
				11.123f,
				11.12f,
				11.1f,
				11.0f,
				10.0f,
				7.0f,
				5.0f,
				3.0f,
				2.0f,
				1.0f,
				0.75f,
				0.71f,
				0.701f,
				0.7f,
				0.5f,
				0.4f,
				0.3f,
				0.2f,
				0.1f,
				0.011f,
				0.0013f,
				0.0012f,
				0.0006f,
				0.0005f,
				0.0004f,
				0.0003f,
				0.0002f,
				0.0001f,
			};

			for (const auto s : scales) {
				for (float32 x = -r; x < r; ++x) {
					//for (float32 y = -r; y < r; ++y) {
					//	auto v = simplex.value(x*s, y*s);
					//	min = v < min ? v : min;
					//	max = v > max ? v : max;
					//}

					auto v = simplex.value1D(x * s);
					min = v < min ? v : min;
					max = v > max ? v : max;

					if (!++p) { ENGINE_INFO("Range: [", min, ", ", max, "] @ ", s); }
				}
			}
			ENGINE_INFO("\n\n===================================================");
			ENGINE_INFO("Range: [", min, ", ", max, "]");
			ENGINE_INFO("===================================================\n");
			std::ios_base::sync_with_stdio(true);
		}/**/

		//using Perm = Engine::Noise::RangePermutation<256>;
		//using Dist = decltype([](auto...){ return 1; });
		//Engine::Noise::WorleyNoiseGeneric<Perm, Dist> worley2{
		//	std::piecewise_construct,
		//	std::forward_as_tuple(1234),
		//	std::forward_as_tuple()
		//};
		

		const Engine::Noise::RangePermutation<256> realPerm = 1234;
		auto perm = [&](auto... as){ return realPerm(as...); };
		//auto dist = [](auto...) { return 1; }; // TODO: cosntexpr SFINAE possible with consteval?
		auto dist = Engine::Noise::ConstantDistribution<1>{};
		auto metric = Engine::Noise::MetricEuclidean2{};
		//auto metric = Engine::Noise::MetricManhattan{};
		//auto metric = Engine::Noise::MetricChebyshev{};
		Engine::Noise::WorleyNoiseGeneric worley2{perm, dist, metric};
		//Engine::Noise::WorleyNoise2 worley2{1234556};
		
		//Engine::Noise::WorleyNoiseFrom<&Engine::Noise::constant1> worley1{1234};
		Engine::Noise::SimplexNoise simplex{(uint64)(srand((uint32)time(0)), rand())};
		Engine::Noise::OpenSimplexNoise simplex2{(uint64)(srand((uint32)time(0)), rand())};
		Game::MapGenerator2 mgen{12345};

		const auto gradient = [](float v, int y, int min, int max, float from, float to){
			if (y < min || y >= max) { return v; }
			float p = static_cast<float>(y - min) / static_cast<float>(max - min); // Get precent
			return v + (p * (to - from) + from); // Map from [0, 1] to [from, to]
		};

		const auto fill = [](float v, int y, int min, int max, float fv){
			if (y < min || y >= max) { return v; }
			return fv;
		};

		Color blockToColor[Game::BlockId::_count] = {};
		blockToColor[Game::BlockId::Entity]	= {0, 120, 189};
		blockToColor[Game::BlockId::Debug]	= {255, 0, 0};
		blockToColor[Game::BlockId::Debug2]	= {200, 26, 226};
		blockToColor[Game::BlockId::Debug3]	= {226, 26, 162};
		blockToColor[Game::BlockId::Debug4]	= {226, 26, 111};
		blockToColor[Game::BlockId::Dirt]	= {158, 98, 33};
		blockToColor[Game::BlockId::Grass]	= {67, 226, 71};
		blockToColor[Game::BlockId::Iron]	= {144, 144, 144};
		blockToColor[Game::BlockId::Gold]	= {255, 235, 65};

		Game::MapGenerator2::BlockGenData discard;

		const auto begin = std::chrono::high_resolution_clock::now();
		for (int y = 0; y < map.h; ++y) {
			for (int x = 0; x < map.w; ++x) {
				const float32 xm = (x - map.w/2) * xZoom + xOffset;
				const float32 ym = (y - map.h/2) * yZoom + yOffset;

				if constexpr (true) {
					const auto v = mgen.value(static_cast<int32>(xm), static_cast<int32>(ym), discard);
					data[y][x] = blockToColor[v];
				}

				if constexpr (false) {
					constexpr float32 ps = 0.03f;
					auto p = 50*simplex2.value(ps*x,ps*y);
					//p += 10*simplex2.value(4*ps*x,4*ps*y);

					glm::vec2 pos = glm::vec2{x+p, y+p};
					constexpr glm::vec2 center = {map.w * 0.5f, map.h * 0.5f};
					const auto diff = center - pos;

					constexpr float32 r = 5.5f;
					const auto off = r*glm::normalize(diff);

					auto v = glm::length(diff);

					float32 f = 0.75f;
					float32 l = 1 / f;
					constexpr float32 i = 32;

					v += i * l * simplex2.value(f * off.x, f * off.y);
					f *= 2.0f;
					l *= 0.5f;
					v += i * l * simplex2.value(f * off.x, f * off.y);
					f *= 2.0f;
					l *= 0.5f;

					v += 30*simplex2.value(0.1f*x, 0.1f*y);
					v += 15*simplex2.value(0.3f*x, 0.3f*y);

					v = (v < 150) * 255.0f;
					//v = v / 300;
					data[y][x].gray(static_cast<uint8>(std::clamp(v, 0.0f, 255.0f)));
				}

				if constexpr (false) {
					const auto off1 = simplex2.value(xm * 0.021f, ym * 0.021f);
					const float32 o1 = 0.4f;
					const auto x2 = xm * 0.01f + off1 * o1;
					const auto y2 = ym * 0.01f + off1 * o1;

					auto m = simplex2.value(xm*0.3f, ym*0.3f);
					//m += simplex2.value(xm*0.15f, ym*0.15f);
					const auto wv = worley2.valueF2F1(x2, y2);
					auto w = wv.value * (1.0f/0.8f);
					auto v = w - m * 0.15f;
					auto a = (uint8)std::clamp(
						(v > 0.2f ? 1.0f : 0.0f)
						* 255.0f
					, 0.0f, 255.0f);
					//w += 0.2f;
					data[y][x].r = perm(wv.cell.x, wv.cell.y) * a;
					data[y][x].g = perm(wv.cell.y, wv.cell.x) * a;
					data[y][x].b = perm(wv.cell.x + 5, wv.cell.y - 5) * a;
				}
				
				//map.data[x] = simplex.value1D(x * 0.1f);
				//map.data2[x] = simplex2.scaled(x * 0.15f, 42.0f);
				//
				//data[y][x].gray((uint8)std::clamp((1 + map.data[x]) * 0.5f * 255.0f, 0.0f, 255.0f));
				//data2[y][x].gray((uint8)std::clamp((1 + map.data2[x]) * 0.5f * 255.0f, 0.0f, 255.0f));
			}
		}
		const auto end = std::chrono::high_resolution_clock::now();

		ENGINE_LOG("Size2: ", sizeof(worley2));
		std::cout << "Map Time (ms): " << std::chrono::duration<long double, std::milli>{end - begin}.count() << "\n";

		glGenTextures(1, &map.tex);
		glBindTexture(GL_TEXTURE_2D, map.tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, map.w, map.h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

		
		//glGenTextures(1, &map.tex2);
		//glBindTexture(GL_TEXTURE_2D, map.tex2);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, map.w, map.h, 0, GL_RGB, GL_UNSIGNED_BYTE, data2);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void mapUI() {
		if (ImGui::Begin("Map Test")) {
			//static glm::vec2 offset = {11481.000, -1485.000};
			//static glm::vec2 zoom = {20, 20};
			static glm::vec2 offset = {8933, -185.000};
			static glm::vec2 zoom = {0.5, 0.5};
			static float scroll = 0;
			static Engine::Clock::TimePoint scrollCooldown = {};

			ImGui::PushItemWidth(256);

			auto buildMap = [&]{
				mapTest(offset.x, offset.y, zoom.x, zoom.y);
			};
			
			ImGui::DragFloat("X Offset", &offset.x, 1.0f);
			if (ImGui::IsItemDeactivatedAfterEdit()) { buildMap(); }

			ImGui::SameLine();

			ImGui::DragFloat("Y Offset", &offset.y, 1.0f);
			if (ImGui::IsItemDeactivatedAfterEdit()) { buildMap(); }
			
			ImGui::DragFloat("X Zoom", &zoom.x, 0.05f, 0.1f, FLT_MAX);
			if (ImGui::IsItemDeactivatedAfterEdit()) { buildMap(); }

			ImGui::SameLine();

			ImGui::DragFloat("Y Zoom", &zoom.y, 0.05f, 0.1f, FLT_MAX);
			if (ImGui::IsItemDeactivatedAfterEdit()) { buildMap(); }

			ImGui::PopItemWidth();

			const glm::vec2 cpos = {ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y};
			constexpr float32 scale = 2;
			if (map.tex == 0) { buildMap(); }
			ImTextureID tid = reinterpret_cast<void*>(static_cast<uintptr_t>(map.tex));
			ImGui::Image(tid, ImVec2(static_cast<float32>(map.w * scale), static_cast<float32>(map.h * scale)), {0,1}, {1,0});

			if (ImGui::IsItemHovered()) {
				const glm::vec2 mpos = {ImGui::GetMousePos().x, ImGui::GetMousePos().y};
				const glm::vec2 moff = mpos - cpos;
				const glm::vec2 isize = glm::vec2{map.w, map.h} * scale;
				const glm::vec2 ioff = glm::vec2{moff.x, -moff.y} - isize * glm::vec2{0.5f, -0.5f};
				const glm::vec2 bpos = glm::vec2{offset.x, offset.y} + ioff * (glm::vec2{zoom.x, zoom.y} / scale);

				ImGui::BeginTooltip();
				//ImGui::Text("%.0f, %.0f", ioff.x, ioff.y);
				ImGui::Text("%.0f, %.0f", bpos.x, bpos.y);
				ImGui::EndTooltip();

				if (const auto s = ImGui::GetIO().MouseWheel) {
					scroll += s;
					scrollCooldown = Engine::Clock::now();
				} else if (scroll && (Engine::Clock::now() - scrollCooldown) >= std::chrono::milliseconds{200}) {
					auto z = std::clamp(scroll * 0.2f, -0.9f, 0.9f); // Limit max zoom
					const auto old = zoom;
					zoom -= zoom * z;
					zoom = glm::max(zoom, 0.05f);
					scroll = 0;
					buildMap();
				}

				constexpr int dragButton = 1;
				if (ImGui::IsMouseReleased(dragButton)) {
					const glm::vec2 delta = {-ImGui::GetMouseDragDelta(dragButton).x, ImGui::GetMouseDragDelta(dragButton).y};
					if (delta.x || delta.y) {
						const glm::ivec2 diff = glm::round(delta * zoom / scale);
						offset += diff;
						buildMap();
					}
				}
			}

			/*
			tid = reinterpret_cast<void*>(static_cast<uintptr_t>(map.tex2));
			ImGui::Image(tid, ImVec2(static_cast<float32>(map.w*2), static_cast<float32>(map.h * 2)), {0,1}, {1,0});
			
			ImPlot::SetNextPlotLimitsY(-1.0f, 1.0f);
			ImPlot::SetNextPlotLimitsX(0, map.w);
			ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2{});
			if (ImPlot::BeginPlot(
				"##noisegraph", nullptr, nullptr, ImVec2(map.w * 2, 200),
				ImPlotFlags_CanvasOnly, ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations)) {
				ImPlot::PlotLine("N1", map.data, (int)std::size(map.data));
				ImPlot::PlotLine("N2", map.data2, (int)std::size(map.data2));
			}
			ImPlot::EndPlot();*/
		}
		ImGui::End();
	}

	void initializeOpenGL() {
		auto loaded = ogl_LoadFunctions();

		if (loaded == ogl_LOAD_FAILED) {
			ENGINE_ERROR("[glLoadGen] initialization failed.");
		}

		auto failed = loaded - ogl_LOAD_SUCCEEDED;
		if (failed > 0) {
			ENGINE_ERROR("[glLoadGen] Failed to load ", failed, " functions.");
		}


		if (!ogl_IsVersionGEQ(OPENGL_VERSION_MAJOR, OPENGL_VERSION_MINOR)) {
			ENGINE_ERROR("[glLoadGen] OpenGL version ", OPENGL_VERSION_MAJOR, ".", OPENGL_VERSION_MINOR, " is not available.");
		}
	}

	b2Body* createPhysicsSquare(Engine::ECS::Entity ent, Game::PhysicsSystem& physSys, b2Vec2 position = b2Vec2_zero) {
		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position = position;

		b2Body* body = physSys.createBody(ent, bodyDef);

		b2PolygonShape shape;
		shape.SetAsBox(1.0f/8, 1.0f/8);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &shape;
		fixtureDef.density = 1.0f;

		body->CreateFixture(&fixtureDef);
		body->SetLinearDamping(10.0f);
		body->SetFixedRotation(true);

		return body;
	}

	
	void performExit(const char* reason) {
		ENGINE_LOG("Shutting down: ", reason, "\n\n");
		fclose(Engine::getGlobalConfig().log.get());
	};
}

namespace {
	// TODO: once input is finished this should go away;
	struct TempWorldEngineWrapper {
		Engine::EngineInstance& engine;
		Game::World& world;
		Engine::Gui::Context& guiContext;
	};

	// TODO: move into file
	class WindowCallbacks final : public Engine::WindowCallbacks<TempWorldEngineWrapper> {
		void resizeCallback(int32 w, int32 h) override {
			ENGINE_LOG("Resize: ", w, " ", h);
			glViewport(0, 0, w, h);
			// blocks per meter - pixels per block - 200% zoom
			userdata->engine.camera.setAsOrtho(w, h, 1.0f / (Game::pixelsPerBlock * Game::blocksPerMeter * 2.0f));
			userdata->guiContext.onResize(w, h);
		}

		void keyCallback(Engine::Input::InputEvent event) override {
			event.state.id.device = 0;
			if (userdata->guiContext.onKey(event)) { return; }
			userdata->world.getSystem<Game::InputSystem>().queueInput(event);
			Engine::ImGui::keyCallback(event.state);
		}

		void charCallback(wchar_t ch) {
			if (userdata->guiContext.onChar(ch)) { return; }
			Engine::ImGui::charCallback(ch);
		}

		void mouseButtonCallback(Engine::Input::InputEvent event) override {
			event.state.id.device = 0;
			if (userdata->guiContext.onMouse(event)) { return; }
			userdata->world.getSystem<Game::InputSystem>().queueInput(event);
			Engine::ImGui::mouseButtonCallback(event.state);
		}

		void mouseWheelCallback(Engine::Input::InputEvent event) override {
			event.state.id.device = 0;
			if (userdata->guiContext.onMouseWheel(event)) { return; }
			userdata->world.getSystem<Game::InputSystem>().queueInput(event);
			Engine::ImGui::scrollCallback(event.state);
		}

		void mouseMoveCallback(Engine::Input::InputEvent event) override {
			event.state.id.device = 0;
			if (userdata->guiContext.onMouseMove(event)) { return; }
			userdata->world.getSystem<Game::InputSystem>().queueInput(event);
			Engine::ImGui::mouseMoveCallback(event.state);
		}

		void mouseLeaveCallback() override {
			userdata->guiContext.onFocus(false);
		}

		void mouseEnterCallback() override {
			userdata->guiContext.onFocus(true);
			Engine::ImGui::mouseEnterCallback();
		}
	};
}

void run(int argc, char* argv[]) {
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Init networking
	////////////////////////////////////////////////////////////////////////////////////////////////
	Engine::Net::startup();

	////////////////////////////////////////////////////////////////////////////////////////////////
	WindowCallbacks windowCallbacks;
	Engine::Window window{{
			.colorBits = 24,
			.alphaBits = 8,
			.depthBits = 24,
			.stencilBits = 8,
		}, {
			.majorVersion = OPENGL_VERSION_MAJOR,
			.minorVersion = OPENGL_VERSION_MINOR,
			#ifdef DEBUG
				.debug = true,
			#else
				.debug = false,
			#endif
		},
		windowCallbacks
	};
	window.makeContextCurrent();
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Initialize OpenGL functions
	////////////////////////////////////////////////////////////////////////////////////////////////
	initializeOpenGL();
	
	// OpenGL debug message
	#if defined(DEBUG)
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(Engine::Debug::GL::debugMessageCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	#endif

	glEnable(GL_FRAMEBUFFER_SRGB);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// UI
	////////////////////////////////////////////////////////////////////////////////////////////////
	IMGUI_CHECKVERSION();
	Engine::ImGui::init(window);
	ImGui::StyleColorsDark();

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Engine
	////////////////////////////////////////////////////////////////////////////////////////////////
	Engine::EngineInstance engine;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Resources
	////////////////////////////////////////////////////////////////////////////////////////////////
	{
		// TODO: these should probably be loaded in a manifest file or something.
		const std::string textures[] = {
			"assets/player.png",
			"assets/fire.png",
			"assets/tree1.png",
			"assets/tree2.png",
			"assets/tree3.png",
			"assets/test.png",
			"assets/test_tree.png",
			"assets/large_sprite_test.png",
			"assets/para_test_0.png",
			"assets/para_test_1.png",
			"assets/para_test_2.png",
			"assets/para_test_outline.png",
		};
		const std::string shaders[] = {
			"shaders/box2d_debug",
			"shaders/terrain",
			"shaders/sprite",
			"shaders/parallax",
			"shaders/gui_poly",
			"shaders/gui_glyph",
			"shaders/fullscreen_passthrough",
		};
		for (const auto& path : textures) { engine.textureManager.add(path); }
		for (const auto& path : shaders) { engine.shaderManager.add(path); }
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// World
	////////////////////////////////////////////////////////////////////////////////////////////////
	auto worldStorage = std::make_unique<Game::World>(engine);
	Game::World& world = *worldStorage.get();
	auto& guiContext = world.getSystem<Game::UISystem>().getContext();
	TempWorldEngineWrapper wrapper{engine, world, guiContext};
	windowCallbacks.userdata = &wrapper;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Binds
	////////////////////////////////////////////////////////////////////////////////////////////////
	const auto updateButtonState = [&](auto action, auto curr){
		world.getSystem<Game::ActionSystem>().updateButtonState(action, curr.i32);
	};
	const auto updateTargetState = [&](auto curr){
		world.getSystem<Game::ActionSystem>().updateTarget(curr.f32v2);
	};
	{
		using namespace Engine::Input;
		auto& im = engine.inputManager;

		if constexpr (ENGINE_CLIENT) {
			im.addBind(0, InputSequence{
				InputId{InputType::KEYBOARD, 0, 29}, // CTRL
				InputId{InputType::KEYBOARD, 0, 46}, // C
				}, [&](Value curr, Value prev){ updateButtonState(Game::Button::Attack1, curr); });
			im.addBind(0, InputSequence{
				InputId{InputType::KEYBOARD, 0, 29}, // CTRL
				InputId{InputType::KEYBOARD, 0, 56}, // ALT
				InputId{InputType::KEYBOARD, 0, 16}, // Q
			}, [&](Value curr, Value prev){ updateButtonState(Game::Button::Attack1, curr); });
			im.addBind(0, InputSequence{
				InputId{InputType::KEYBOARD, 0, 57}
			}, [&](Value curr, Value prev){ updateButtonState(Game::Button::Attack1, curr); });
			im.addBind(0, InputSequence{
				InputId{InputType::KEYBOARD, 0, 17}
			}, [&](Value curr, Value prev){ updateButtonState(Game::Button::MoveUp, curr); });
			im.addBind(0, InputSequence{
				InputId{InputType::KEYBOARD, 0, 31}
			}, [&](Value curr, Value prev){ updateButtonState(Game::Button::MoveDown, curr); });
			im.addBind(0, InputSequence{
				InputId{InputType::KEYBOARD, 0, 30}
			}, [&](Value curr, Value prev){ updateButtonState(Game::Button::MoveLeft, curr); });
			im.addBind(0, InputSequence{
				InputId{InputType::KEYBOARD, 0, 32}
			}, [&](Value curr, Value prev){ updateButtonState(Game::Button::MoveRight, curr); });

			im.addBind(0, InputSequence{
				InputId{InputType::MOUSE, 0, 0}
			}, [&](Value curr, Value prev){ updateButtonState(Game::Button::Attack1, curr); });
			im.addBind(0, InputSequence{
				InputId{InputType::MOUSE, 0, 1}
			}, [&](Value curr, Value prev){ updateButtonState(Game::Button::Attack2, curr); });

			im.addBind(0, InputSequence{
					InputId{InputType::MOUSE_AXIS, 0, 0}
				}, [&](Value curr, Value prev){
					updateTargetState(curr);
			});
		}

		//as.addListener(Spell_1, Game::CharacterSpellActionListener{engine, world});
		//as.addListener(Move_Up, Game::CharacterMovementActionListener{world, glm::ivec2{0, 1}});
		//as.addListener(Move_Down, Game::CharacterMovementActionListener{world, glm::ivec2{0, -1}});
		//as.addListener(Move_Left, Game::CharacterMovementActionListener{world, glm::ivec2{-1, 0}});
		//as.addListener(Move_Right, Game::CharacterMovementActionListener{world, glm::ivec2{1, 0}});
		//as.addListener(Edit_Remove, [&](Engine::ECS::Entity ent, ActionId, Value curr, Value prev){ world.getComponent<Game::MapEditComponent>(ent).remove = curr && !prev; return false; });
		//as.addListener(Edit_Place, [&](Engine::ECS::Entity ent, ActionId, Value curr, Value prev){ world.getComponent<Game::MapEditComponent>(ent).place = curr && !prev; return false; });
	}

	// More engine stuff
	#if defined (DEBUG_PHYSICS)
		world.getSystem<Game::PhysicsSystem>().getDebugDraw().setup(engine.camera);
	#endif
	
	// Main loop
	std::array<float, 64> deltas = {};
	size_t deltaIndex = 0;

	if constexpr (ENGINE_SERVER) {
		window.setPosSize(3440, 0, 1920, 1080);
	} else {
		//window.setClientArea(1920, 1080);
		//window.center();
		window.setClientArea(1920, 1080);
		window.setPosition(1250, 100);
	}

	window.setSwapInterval(0);
	window.show();

	{
		const auto endTime = Engine::Clock::now();
		ENGINE_INFO("Launch Time: ", Engine::Clock::Milliseconds{endTime - launchTime}.count(), "ms");
		ENGINE_INFO("Start Time: ", Engine::Clock::Milliseconds{endTime - startTime}.count(), "ms");
	}

	glClearColor(0.2176f, 0.2176f, 0.2176f, 1.0f);
	while (!window.shouldClose()) {
		window.poll();

		// Rendering
		glClear(GL_COLOR_BUFFER_BIT);
		Engine::ImGui::newFrame();

		// ECS
		world.run();

		// Frame rate
		deltas[deltaIndex] = world.getDeltaTime();
		deltaIndex = ++deltaIndex % deltas.size();
		if (deltaIndex == 0) {
			avgDeltaTime = std::accumulate(deltas.cbegin(), deltas.cend(), 0.0) / deltas.size();
		}

		// Physics debug
		#if defined (DEBUG_PHYSICS)
			world.getSystem<Game::PhysicsSystem>().getDebugDraw().draw();
		#endif

		if constexpr (ENGINE_CLIENT) { mapUI(); }

		guiContext.render();
		Engine::ImGui::draw();
		window.swapBuffers();
		//std::this_thread::sleep_for(std::chrono::milliseconds{250});
	}

	glDeleteTextures(1, &map.tex);
	//glDeleteTextures(1, &map.tex2);

	// UI cleanup
	Engine::ImGui::shutdown();

	// Network cleanup
	Engine::Net::shutdown();
}

static_assert(ENGINE_CLIENT ^ ENGINE_SERVER, "Must be either client or server");
int entry(int argc, char* argv[]) {
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Command Line
	////////////////////////////////////////////////////////////////////////////////////////////////
	Engine::CommandLine::Parser parser;

	{
		using namespace Engine::CommandLine;
		using namespace Engine::Net;
		//auto& parser = ;

		parser
			.add<uint16>("port", 'p', 21212,
				"The port to listen on.")
			.add<IPv4Address>("group", 'g', {224,0,0,212, 21212},
				"The multicast group to join for server discovery.")
			.add<std::string>("log", 'l', "",
				"The file to use for logging.")
			.add<bool>("logColor",
				"Enable or disable color log output.")
			.add<bool>("logTimeOnly",
				"Show only time stamps instead of full dates in log output.")
		;

		parser.parse(argc - 1, argv + 1);
		auto& cfg = Engine::getGlobalConfig<true>();

		{ // Setup first part of global log config
			const auto* log = parser.get<std::string>("log");

			if (log && !log->empty()) {
				std::cout << "Log file: " << *log << std::endl;
				auto old = std::move(cfg.log);
				cfg.log = {fopen(log->c_str(), "ab+"), &fclose};

				if (!cfg.log) {
					cfg.log = std::move(cfg.log);
					ENGINE_WARN("Failed to open log file: ", *log);
				}
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Configure Console
	////////////////////////////////////////////////////////////////////////////////////////////////
	if(!AllocConsole()) {
		ENGINE_WARN("Unable to allocate console window - ", Engine::Win32::getLastErrorMessage());
	} else {
		FILE* unused;
		freopen_s(&unused, "CONIN$", "r", stdin);
		freopen_s(&unused, "CONOUT$", "w", stdout);
		freopen_s(&unused, "CONOUT$", "w", stderr);
	}

	
	if (auto console = GetStdHandle(STD_OUTPUT_HANDLE);
		!console || !SetConsoleMode(console, ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
		ENGINE_WARN(Engine::Win32::getLastErrorMessage());
	}

	// Position the console
	if (HWND window; window = GetConsoleWindow()) {
		if constexpr (ENGINE_CLIENT) {
			SetWindowPos(window, HWND_TOP, 0, 0, 1500, 500, 0);
			SetWindowTextW(window, L"Client");
		} else if constexpr (ENGINE_SERVER) {
			SetWindowPos(window, HWND_TOP, 0, 500, 1500, 500, 0);
			SetWindowTextW(window, L"Server");
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Setup global config
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Win32: This must be after AllocConsole and SetConsoleMode or else isatty wont work
	{ 
		auto& cfg = Engine::getGlobalConfig<true>();

		{ // Setup second part of global log config
			const auto* logColor = parser.get<bool>("logColor");
			const auto* logTimeOnly = parser.get<bool>("logTimeOnly");
			const bool isTerminal = isatty(fileno(cfg.log.get()));
			cfg.logColor = logColor ? *logColor : isTerminal;
			cfg.logTimeOnly = logTimeOnly ? *logTimeOnly : isTerminal;
		}

		{ // Setup global network config
			const auto* port = parser.get<uint16>("port");
			if (port) { cfg.port = *port; }

			const auto* group = parser.get<Engine::Net::IPv4Address>("group");
			if (group) { cfg.group = *group; }
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Other
	////////////////////////////////////////////////////////////////////////////////////////////////
	ENGINE_INFO("Starting ", ENGINE_SERVER ? "Server" : "Client");
	ENGINE_INFO("Working Directory: ", std::filesystem::current_path().generic_string());

	signal(SIGABRT, [](int){ performExit("abort signal received"); });
	signal(SIGTERM, [](int){ performExit("terminate signal received"); });

	#ifdef ENGINE_OS_WINDOWS
		SetConsoleCtrlHandler([](DWORD ctrlType) -> BOOL {
			const char* type = nullptr;

			switch(ctrlType) {
				case CTRL_C_EVENT: { type = "CTRL_C_EVENT"; break; }
				case CTRL_BREAK_EVENT: { type = "CTRL_BREAK_EVENT"; break; }
				case CTRL_CLOSE_EVENT: { type = "CTRL_CLOSE_EVENT"; break; }
				case CTRL_LOGOFF_EVENT: { type = "CTRL_LOGOFF_EVENT"; break; }
				case CTRL_SHUTDOWN_EVENT: { type = "CTRL_SHUTDOWN_EVENT"; break; }
			}

			if (type) {
				std::string msg = "win32 control handler called with ";
				msg += type;
				performExit(msg.c_str());
			}

			return false;
		}, true);
	#endif

	std::atexit([]{ performExit("exit requested"); });
	std::at_quick_exit([]{ performExit("quick exit requested"); });
	std::set_terminate([]{ performExit("terminate handler called"); });
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Run
	////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// At seemingly random the debugger decides to not work. Enable, run, disable, run seems to fix this for some reason.
	// Other times spam clicking on Visual Studio while the program launches fixes this. 10/10.
	// Minimizing the server window before crash seems to help
	// --------------------------
	// Function Breakpoints: assert, _wassert, abort, exit
	// --------------------------
	//_set_error_mode(_OUT_TO_MSGBOX);
	//_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_WNDW | _CRTDBG_MODE_DEBUG);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//Engine::ConfigParser cfg;
	//cfg.loadAndTokenize("example.cfg");
	//cfg.print();

	startTime = Engine::Clock::now();
	run(argc, argv);

	ENGINE_LOG("Done.");
	return EXIT_SUCCESS;
}

#ifdef ENGINE_OS_WINDOWS
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow) {
	launchTime = Engine::Clock::now();
	return entry(__argc, __argv);
}
#else
int main(int argc, char* argv[]) {
	return entry(argc, argv);
}
#endif
