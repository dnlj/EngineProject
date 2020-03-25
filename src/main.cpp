// Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// STD
#include <algorithm>
#include <iostream>
#include <thread>
#include <cmath>
#include <numeric>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// SOIL
#include <soil/SOIL.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Meta
#include <Meta/TypeSet/TypeSet.hpp>

// ImGui
#include <imgui.h>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Debug/Debug.hpp>
#include <Engine/TextureManager.hpp>
#include <Engine/Utility/Utility.hpp>
#include <Engine/ECS/World.hpp>
#include <Engine/EngineInstance.hpp>
#include <Engine/Camera.hpp>
#include <Engine/ResourceManager.hpp>
#include <Engine/Noise/OpenSimplexNoise.hpp>
#include <Engine/Noise/WorleyNoise.hpp>
#include <Engine/Net/Net.hpp>
#include <Engine/Net/UDPSocket.hpp>
#include <Engine/Windows/Windows.hpp>
#include <Engine/Windows/OpenGLWindow.hpp>


// Game
#include <Game/Common.hpp>
#include <Game/World.hpp>
#include <Game/CharacterSpellBindListener.hpp>
#include <Game/CharacterMovementBindListener.hpp>
#include <Game/MapGenerator.hpp>
#include <Game/MapSystemBindListener.hpp>
#include <Game/imgui_impl_glfw_gl3.hpp>


namespace {
	constexpr int32 OPENGL_VERSION_MAJOR = 4;
	constexpr int32 OPENGL_VERSION_MINOR = 5;
	GLuint mapTexture = 0;
	double avgDeltaTime = 0.0;
	
	void mapTest() {
		struct Color {
			uint8_t r = 255;
			uint8_t g = 0;
			uint8_t b = 0;

			Color() = default;
			explicit Color(uint32_t value) {
				*this = value;
			}

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

		constexpr int w = 512;
		constexpr int h = 512;
		//constexpr int w = 128;
		//constexpr int h = 128;

		Color map[h][w];

		Engine::Noise::OpenSimplexNoise simplex{1234};
		Engine::Noise::WorleyNoise worley{1234};
		Engine::Noise::WorleyNoiseFrom<&Engine::Noise::constant1> worley1{1234};
		Game::MapGenerator<
			Game::BiomeA,
			//Game::BiomeB,
			Game::BiomeC
		> mgen{1234};

		/*{
			srand((unsigned)time(NULL));
			Engine::Noise::WorleyNoise worley{rand()};
			int w = 1 << 10;
			int h = w;
			double max = 0;
			double min = 0;
			for (int y = 0; y < h; ++y) {
				for (int x = 0; x < w; ++x) {
					const auto v = (double)worley.at((float)x,(float)y);
					max = std::max(max, v);
					min = std::min(min, v);
				}
			}

			// 1.08485
			std::cout << "Max: " << max << "\n";
			std::cout << "Min: " << min << "\n";
		}*/


		const auto gradient = [](float v, int y, int min, int max, float from, float to){
			if (y < min || y >= max) { return v; }
			float p = static_cast<float>(y - min) / static_cast<float>(max - min); // Get precent
			return v + (p * (to - from) + from); // Map from [0, 1] to [from, to]
		};

		const auto fill = [](float v, int y, int min, int max, float fv){
			if (y < min || y >= max) { return v; }
			return fv;
		};

		const auto begin = std::chrono::high_resolution_clock::now();
		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) {
				float v = 0.0f;
				
				{ // Cave structure
					float s = 0.051f;

					//s *= 2;
					//v += simplex.value(x * s, y * s) / 2;
					
					//s *= 2;
					//v += simplex.value(x * s, y * s) / 4;
					//
					//s *= 2;
					//v += simplex.value(x * s, y * s) / 8;
					//
					//s *= 2;
					//v += simplex.value(x * s, y * s) / 16;
				}

				/*
				{ // Surface
					constexpr int begin = 100;
					constexpr int mid = begin + 40;
					constexpr int end = mid + 30;
					v = gradient(v, y, begin, mid, -1.0f, 1.0f);
					v = gradient(v, y, mid, end, 1.0f, 0.0f);
					v = fill(v, y, 0, begin, -1.0f);
				}
				*/
				
				{ // Worley testing
					float s = 0.01f;
					//v = sqrt(worley.value(x * s, y * s).distanceSquared);
					//v = sqrt(worley.valueD2(x * s, y * s).value);
					//v = worley1.valueF2F1(x * s, y * s).value;
					v = mgen.value(x, y);
				}

				// Step
				v = v < 0.0f ? -1.0f : 1.0f;

				// Convert to color map
				const auto y2 = h - y - 1;
				map[y2][x].gray(static_cast<uint8_t>(roundf(std::max(std::min(
					(v + 1.0f) * 0.5f * 255.0f
					//v * 255.0f
					//(1 - v) * 255.0f
				, 255.0f), 0.0f))));

				if (x < 10 && y < 20) {
					map[y2][x].r = 255;
					map[y2][x].g = 0;
					map[y2][x].b = 0;
				}
			}
		}

		const auto end = std::chrono::high_resolution_clock::now();

		std::cout << "Map Time (ms): " << std::chrono::duration<long double, std::milli>{end - begin}.count() << "\n";

		glGenTextures(1, &mapTexture);
		glBindTexture(GL_TEXTURE_2D, mapTexture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, map);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void editorUI(Engine::EngineInstance& engine, Game::World& world) {
		static auto texture32 = engine.textureManager.get("../assets/32.bmp");

		bool open = true;
		ImGui::Begin("Editor UI", &open, ImGuiWindowFlags_MenuBar);

		ImGui::Text("FPS %f (%f)", 1.0/avgDeltaTime, avgDeltaTime);

		if (ImGui::CollapsingHeader("Debug", ImGuiTreeNodeFlags_DefaultOpen)) {
			auto& mapSys = world.getSystem<Game::MapSystem>();

			auto screenMousePos = engine.inputManager.getMousePosition();
			ImGui::Text("Mouse (screen): (%f, %f)", screenMousePos.x, screenMousePos.y);

			auto worldMousePos = engine.camera.screenToWorld(screenMousePos);
			ImGui::Text("Mouse (world): (%f, %f)", worldMousePos.x, worldMousePos.y);
			
			auto blockMousePos = mapSys.worldToBlock(worldMousePos);
			ImGui::Text("Mouse (block): (%i, %i)", blockMousePos.x, blockMousePos.y);

			auto blockWorldMousePos = mapSys.blockToWorld(blockMousePos);
			ImGui::Text("Mouse (block-world): (%f, %f)", blockWorldMousePos.x, blockWorldMousePos.y);

			auto chunkMousePos = mapSys.blockToChunk(blockMousePos);
			auto chunkBlockMousePos = mapSys.chunkToBlock(chunkMousePos);
			ImGui::Text("Mouse (chunk): (%i, %i) (%i, %i)", chunkMousePos.x, chunkMousePos.y, chunkBlockMousePos.x, chunkBlockMousePos.y);

			const auto regionMousePos = mapSys.chunkToRegion(chunkMousePos);
			ImGui::Text("Mouse (region): (%i, %i)", regionMousePos.x, regionMousePos.y);
			
			auto camPos = engine.camera.getPosition();
			ImGui::Text("Camera: (%f, %f, %f)", camPos.x, camPos.y, camPos.z);

			auto mapOffset = world.getSystem<Game::PhysicsOriginShiftSystem>().getOffset();
			ImGui::Text("Map Offset: (%i, %i)", mapOffset.x, mapOffset.y);

			auto mapBlockOffset = mapSys.getBlockOffset();
			ImGui::Text("Map Offset (block): (%i, %i)", mapBlockOffset.x, mapBlockOffset.y);

			auto mapChunkOffset = mapSys.blockToChunk(mapBlockOffset);
			ImGui::Text("Map Offset (chunk): (%i, %i)", mapChunkOffset.x, mapChunkOffset.y);


			#if defined(DEBUG_PHYSICS)
				auto& physDebug = world.getSystem<Game::PhysicsSystem>().getDebugDraw();
				ImGui::Text("Physics Debug Verts: (%i)", physDebug.getVertexCount());
			#endif
		}

		if (ImGui::CollapsingHeader("Map")) {
			ImGui::Image(reinterpret_cast<void*>(static_cast<uintptr_t>(mapTexture)), ImVec2(1024, 1024));
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

	void doUI(Engine::EngineInstance& engine, Game::World& world) {
		static bool showWindow = true;
		ImGui_ImplGlfwGL3_NewFrame();
		ImGui::ShowDemoWindow(&showWindow);
		editorUI(engine, world);
		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
	}

	b2Body* createPhysicsCircle(Engine::ECS::Entity ent, Game::PhysicsSystem& physSys, b2Vec2 position = b2Vec2_zero) {
		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position = position;

		b2Body* body = physSys.createBody(ent, bodyDef);

		b2CircleShape shape;
		shape.m_radius = 1.0f/8;

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &shape;
		fixtureDef.density = 1.0f;

		body->CreateFixture(&fixtureDef);
		body->SetLinearDamping(10.0f);
		body->SetFixedRotation(true);

		return body;
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
}

void run() {
	// Init networking
	Engine::Net::startup();

	// Network test
	//{
	//	#if ENGINE_SERVER
	//		Engine::Net::UDPSocket socket{27015};
	//	#else
	//		Engine::Net::UDPSocket socket;
	//	#endif
	//	
	//	while (true) {
	//		if constexpr (ENGINE_CLIENT) {
	//			constexpr char data[] = "Hello, world!";
	//			socket.send({127, 0, 0, 1, 27015}, data, sizeof(data));
	//		} else {
	//			socket.recv();
	//		}
	//		Sleep(1000);
	//	}
	//}

	////////////////////////////////////////////////////////////////////////////////////////////////
	Engine::Windows::OpenGLWindow window{{
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
	}};
	window.makeContextCurrent();

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Initialize OpenGL functions
	initializeOpenGL();

	// OpenGL debug message
	#if defined(DEBUG_GRAPHICS)
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(Engine::Debug::GL::debugMessageCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	#endif

	glEnable(GL_FRAMEBUFFER_SRGB);
	////////////////////////////////////////////////////////////////////////////////////////////////
	// UI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfwGL3_Init(window.getWin32WindowHandle());
	ImGui::StyleColorsDark();

	////////////////////////////////////////////////////////////////////////////////////////////////
	// TODO: once input is finished this should go away;
	struct TempWorldEngineWrapper {
		Engine::EngineInstance& engine;
		Game::World& world;
	};

	window.setKeyPressCallback([](void* userdata, int scancode, bool extended){
		//puts("keyPressCallback");
		auto& wrapper = *static_cast<TempWorldEngineWrapper*>(userdata);
		wrapper.world.getSystem<Game::InputSystem>().queueInput({{Engine::Input::InputType::KEYBOARD, scancode}, true});
		ImGui_ImplGlfw_KeyCallback(scancode, true);
	});

	window.setKeyReleaseCallback([](void* userdata, int scancode, bool extended){
		//puts("keyReleaseCallback");
		auto& wrapper = *static_cast<TempWorldEngineWrapper*>(userdata);
		wrapper.world.getSystem<Game::InputSystem>().queueInput({{Engine::Input::InputType::KEYBOARD, scancode}, false});
		ImGui_ImplGlfw_KeyCallback(scancode, false);
	});

	window.setCharCallback([](void* userdata, wchar_t character){
		//puts("setCharCallback");
		auto& wrapper = *static_cast<TempWorldEngineWrapper*>(userdata);
		//wrapper.world.getSystem<Game::InputSystem>().queueInput({{Engine::Input::InputType::KEYBOARD, scancode}, true});
		ImGui_ImplGlfw_CharCallback(character);
	});

	window.setMousePressCallback([](void* userdata, int32 button){
		//puts("mousePressCallback");
		auto& wrapper = *static_cast<TempWorldEngineWrapper*>(userdata);
		wrapper.world.getSystem<Game::InputSystem>().queueInput({{Engine::Input::InputType::MOUSE, button}, true});
		ImGui_ImplGlfw_MouseButtonCallback(button, true);
	});

	window.setMouseReleaseCallback([](void* userdata, int32 button){
		//puts("mouseReleaseCallback");
		auto& wrapper = *static_cast<TempWorldEngineWrapper*>(userdata);
		wrapper.world.getSystem<Game::InputSystem>().queueInput({{Engine::Input::InputType::MOUSE, button}, false});
		ImGui_ImplGlfw_MouseButtonCallback(button, false);
	});

	window.setMouseMoveCallback([](void* userdata, int32 x, int32 y){
		//puts("mouseMoveCallback");
		//std::cout << "Mouse Pos: " << x << ", " << y << "\n";
		auto& wrapper = *static_cast<TempWorldEngineWrapper*>(userdata);
		wrapper.engine.inputManager.mouseCallback(x, y);
		// TODO: inputsystem
	});

	window.setResizeCallback([](void* userdata, int32 w, int32 h){
		//puts("sizingCallback");
		//std::cout << w << " " << h << "\n";
		glViewport(0, 0, w, h);
		auto& wrapper = *static_cast<TempWorldEngineWrapper*>(userdata);
		wrapper.engine.camera.setAsOrtho(w, h, 1.0f / 250.0f);
	});

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Engine stuff
	Engine::EngineInstance engine;
	auto worldStorage = std::make_unique<Game::World>(1.0f / 60.0f, engine);
	Game::World& world = *worldStorage.get();
	TempWorldEngineWrapper wrapper{engine, world};
	window.userdata = &wrapper;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Binds
	{
		using namespace Engine::Input;
		engine.inputManager.addInputBindMapping("Spell_1", InputSequence{
			InputId{InputType::KEYBOARD, 29}, // CTRL
			InputId{InputType::KEYBOARD, 46}, // C
		});
		engine.inputManager.addInputBindMapping("Spell_1", InputSequence{
			InputId{InputType::KEYBOARD, 29}, // CTRL
			InputId{InputType::KEYBOARD, 56}, // ALT
			InputId{InputType::KEYBOARD, 16}, // Q
		});
		engine.inputManager.addInputBindMapping("Spell_1", InputSequence{
			InputId{InputType::KEYBOARD, 57}
		});
		engine.inputManager.addInputBindMapping("MoveUp", InputSequence{
			InputId{InputType::KEYBOARD, 17}
		});
		engine.inputManager.addInputBindMapping("MoveDown", InputSequence{
			InputId{InputType::KEYBOARD, 31}
		});
		engine.inputManager.addInputBindMapping("MoveLeft", InputSequence{
			InputId{InputType::KEYBOARD, 30}
		});
		engine.inputManager.addInputBindMapping("MoveRight", InputSequence{
			InputId{InputType::KEYBOARD, 32}
		});
		engine.inputManager.addInputBindMapping("EditPlace", InputSequence{
			InputId{InputType::MOUSE, 0}
		});
		engine.inputManager.addInputBindMapping("EditRemove", InputSequence{
			InputId{InputType::MOUSE, 1}
		});
	}

	// More engine stuff
	#if defined (DEBUG_PHYSICS)
		world.getSystem<Game::PhysicsSystem>().getDebugDraw().setup(engine.camera);
	#endif

	auto player = world.createEntity();

	// TODO: better place to store these?
	Game::CharacterSpellBindListener playerSpellBindListener{engine, world, player};
	Game::CharacterMovementBindListener playerMovementBindListeners[] = {
		Game::CharacterMovementBindListener{world, player, glm::ivec2{0, 1}},
		Game::CharacterMovementBindListener{world, player, glm::ivec2{0, -1}},
		Game::CharacterMovementBindListener{world, player, glm::ivec2{-1, 0}},
		Game::CharacterMovementBindListener{world, player, glm::ivec2{1, 0}},
	};
	Game::MapSystemBindListener<1> mapSystemBindListener_EditPlace{world.getSystem<Game::MapSystem>(), engine};
	Game::MapSystemBindListener<0> mapSystemBindListener_EditRemove{world.getSystem<Game::MapSystem>(), engine};

	{ // TODO: is there a better way to handle these setup functions? This seems dumb.
		auto& physSys = world.getSystem<Game::PhysicsSystem>();
		
		// Player
		world.addComponent<Game::SpriteComponent>(player).texture = engine.textureManager.get("../assets/player.png");
		world.addComponent<Game::PhysicsComponent>(player).setBody(createPhysicsCircle(player, physSys));
		world.addComponent<Game::CharacterMovementComponent>(player);
		world.addComponent<Game::CharacterSpellComponent>(player);
		world.addComponent<Game::InputComponent>(player).inputManager = &engine.inputManager;

		world.getSystem<Game::CameraTrackingSystem>().focus = player;

		// TODO: Do this in a better way? Listener on an EntityFilter for CharacterMovementComponent would be one way.
		engine.inputManager.getBind("Spell_1").addListener(&playerSpellBindListener);
		engine.inputManager.getBind("MoveUp").addListener(&playerMovementBindListeners[0]);
		engine.inputManager.getBind("MoveDown").addListener(&playerMovementBindListeners[1]);
		engine.inputManager.getBind("MoveLeft").addListener(&playerMovementBindListeners[2]);
		engine.inputManager.getBind("MoveRight").addListener(&playerMovementBindListeners[3]);
		engine.inputManager.getBind("EditPlace").addListener(&mapSystemBindListener_EditPlace);
		engine.inputManager.getBind("EditRemove").addListener(&mapSystemBindListener_EditRemove);

	}

	//// ImGui callbacks
	// TODO: glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
	// TODO: glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);

	// Procedural test
	//mapTest();

	// Main loop
	std::array<float, 64> deltas = {};
	size_t deltaIndex = 0;
	window.show();
	while (!window.shouldClose()) {
		window.poll();

		// Input
		engine.inputManager.update();

		// Rendering
		glClearColor(0.2176f, 0.2176f, 0.2176f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

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

		// TODO: move into system
		// Draw UI
		doUI(engine, world);

		window.swapBuffers();
		//std::this_thread::sleep_for(std::chrono::milliseconds{250});
	}

	glDeleteTextures(1, &mapTexture);

	// UI cleanup
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();

	// Network cleanup
	Engine::Net::shutdown();
}

static_assert(ENGINE_CLIENT ^ ENGINE_SERVER, "Must be either client or server");
int wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	if(!AllocConsole()) {
		ENGINE_ERROR("Unable to allocate console window - ", Engine::Windows::getLastErrorMessage());
	} else {
		FILE* unused;
		freopen_s(&unused, "CONIN$", "r", stdin);
		freopen_s(&unused, "CONOUT$", "w", stdout);
		freopen_s(&unused, "CONOUT$", "w", stderr);
	}

	std::atexit([](){
	});

	{ // Position the console
		auto window = GetConsoleWindow();
		if constexpr (ENGINE_CLIENT) {
			SetWindowPos(window, HWND_TOP, 0, 0, 1000, 500, 0);
			SetWindowTextW(window, L"Client");
		} else if (ENGINE_SERVER) {
			SetWindowPos(window, HWND_TOP, 0, 500, 1000, 500, 0);
			SetWindowTextW(window, L"Server");
		}
	}

	run();

	std::cout << "Done." << std::endl;
	return EXIT_SUCCESS;
}
