// Windows
#include <Windows.h>

// STD
#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// GLFW
#include <GLFW/glfw3.h>

// SOIL
#include <SOIL.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Meta
#include <Meta/TypeSet/TypeSet.hpp>

// ImGui
#include <imgui.h>

// OpenSimplexNoise
#include <OpenSimplexNoise.hpp>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Debug/Debug.hpp>
#include <Engine/SystemBase.hpp>
#include <Engine/TextureManager.hpp>
#include <Engine/Utility/Utility.hpp>
#include <Engine/ECS/World.hpp>
#include <Engine/EngineInstance.hpp>
#include <Engine/Camera.hpp>
#include <Engine/ResourceManager.hpp>

// Game
#include <Game/Common.hpp>
#include <Game/SpriteSystem.hpp>
#include <Game/CameraTrackingSystem.hpp>
#include <Game/CharacterSpellSystem.hpp>
#include <Game/CharacterSpellBindListener.hpp>
#include <Game/CharacterSpellComponent.hpp>
#include <Game/CharacterMovementSystem.hpp>
#include <Game/CharacterMovementBindListener.hpp>
#include <Game/CharacterMovementComponent.hpp>
#include <Game/MapSystem.hpp>
#include <Game/MapSystemBindListener.hpp>
#include <Game/SpriteComponent.hpp>
#include <Game/InputComponent.hpp>
#include <Game/PhysicsComponent.hpp>
#include <Game/imgui_impl_glfw_gl3.hpp>


namespace {
	constexpr int OPENGL_VERSION_MAJOR = 4;
	constexpr int OPENGL_VERSION_MINOR = 5;
	GLuint mapTexture = 0;

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

		Color map[h][w];

		OpenSimplexNoise noise{1234};

		const auto gradient = [](float v, int y, int min, int max, float from, float to){
			if (y < min || y >= max) { return v; }
			float p = static_cast<float>(y - min) / static_cast<float>(max - min); // Get precent
			return v + (p * (to - from) + from); // Map from [0, 1] to [from, to]
		};

		const auto fill = [](float v, int y, int min, int max, float fv){
			if (y < min || y >= max) { return v; }
			return fv;
		};

		for (int y = 0; y < h; ++y) {
			for (int x = 0; x < w; ++x) {
				float v = 0.0f;
				float s = 0.02f;

				s *= 2;
				v += noise.eval(x * s, y * s) / 2;

				s *= 2;
				v += noise.eval(x * s, y * s) / 4;
				
				s *= 2;
				v += noise.eval(x * s, y * s) / 8;
				
				s *= 2;
				v += noise.eval(x * s, y * s) / 16;

				v = gradient(v, y, 64, 128, -1.0f, 1.0f);
				v = gradient(v, y, 128, 200, 1.0f, 0.0f);
				v = fill(v, y, 0, 64, -1.0f);

				v = v < 0.0f ? -1.0f : 1.0f; // TODO: try having a gradient for the step value

				v = std::max(std::min(v, 1.0f), -1.0f);
				map[y][x].gray(static_cast<uint8_t>(roundf(
					(v + 1.0f) * 0.5f * 255.0f
				)));
			}
		}

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

		if (ImGui::CollapsingHeader("Debug")) {
			auto screenMousePos = engine.inputManager.getMousePosition();
			ImGui::Text("Mouse (screen): (%f, %f)", screenMousePos.x, screenMousePos.y);

			auto worldMousePos = engine.camera.screenToWorld(screenMousePos);
			ImGui::Text("Mouse (world): (%f, %f)", worldMousePos.x, worldMousePos.y);

			auto camPos = engine.camera.getPosition();
			ImGui::Text("Camera: (%f, %f, %f)", camPos.x, camPos.y, camPos.z);

			auto& mapSys = world.getSystem<Game::MapSystem>();
			auto mapOffset = mapSys.getOffset();
			ImGui::Text("Map Offset: (%i, %i)", mapOffset.x, mapOffset.y);

			#if defined(DEBUG_PHYSICS)
				auto& physDebug = world.getSystem<Game::PhysicsSystem>().getDebugDraw();
				ImGui::Text("Physics Debug Verts: (%i)", physDebug.getVertexCount());
			#endif
		}

		if (ImGui::CollapsingHeader("Map", ImGuiTreeNodeFlags_DefaultOpen)) {
			auto screenMousePos = engine.inputManager.getMousePosition();
			ImGui::Text("Mouse (screen): (%f, %f)", screenMousePos.x, screenMousePos.y);
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
			ENGINE_ERROR("[glLoadGen] Failed to load " << failed << " functions.");
		}


		if (!ogl_IsVersionGEQ(OPENGL_VERSION_MAJOR, OPENGL_VERSION_MINOR)) {
			ENGINE_ERROR("[glLoadGen] OpenGL version " << OPENGL_VERSION_MAJOR << "." << OPENGL_VERSION_MINOR << " is not available.");
		}
	}

	GLFWwindow* createWindow() {
		// GLFW hints
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		#if defined(DEBUG)
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		#endif
		
		glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
		glfwWindowHint(GLFW_DECORATED, GL_TRUE);
		glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

		glfwWindowHint(GLFW_RED_BITS, 8);
		glfwWindowHint(GLFW_GREEN_BITS, 8);
		glfwWindowHint(GLFW_BLUE_BITS, 8);
		glfwWindowHint(GLFW_ALPHA_BITS, 8);
		glfwWindowHint(GLFW_DEPTH_BITS, 32);

		// Create a window
		constexpr int width = 1900;
		constexpr int height = 1300;
		auto window = glfwCreateWindow(width, height, "Window Title", nullptr, nullptr);

		if (!window) {
			ENGINE_ERROR("[GLFW] Failed to create window.");
		}

		{ // Position the window
			auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			glfwSetWindowPos(window, mode->width/2 - width/2, mode->height/2 - height/2);
		}

		return window;
	}

	void initImGui(GLFWwindow* window) {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui_ImplGlfwGL3_Init(window, false);
		ImGui::StyleColorsDark();
	}

	void doUI(Engine::EngineInstance& engine, Game::World& world) {
		static bool showWindow = true;

		ImGui_ImplGlfwGL3_NewFrame();

		//ImGui::ShowDemoWindow(&showWindow);
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

	void framebufferCallback(GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);

		auto& camera = static_cast<Engine::EngineInstance*>(glfwGetWindowUserPointer(window))->camera;
		camera.setAsOrtho(width, height, 1.0f / 250.0f);
	}
}

void run() {
	// GLFW error callback
	glfwSetErrorCallback([](int error, const char* desc) {
		ENGINE_ERROR("[GLFW] " << desc);
	});

	// Initialize GLFW
	if (!glfwInit()) {
		ENGINE_ERROR("[GLFW] Failed to initialize.");
	}

	// Create a window
	GLFWwindow* window = createWindow();
	glfwMakeContextCurrent(window);
	
	// Enable vsync
	glfwSwapInterval(1);

	// Initialize OpenGL functions
	initializeOpenGL();

	// OpenGL debug message
	#if defined(DEBUG_GRAPHICS)
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(Engine::Debug::GL::debugMessageCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	#endif

	glEnable(GL_FRAMEBUFFER_SRGB);

	// UI
	initImGui(window);

	// Engine stuff
	Engine::EngineInstance engine;
	Game::World world;

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
	Game::MapSystemBindListener<&Game::MapChunk::addTile> mapSystemBindListener_EditPlace{world.getSystem<Game::MapSystem>()};
	Game::MapSystemBindListener<&Game::MapChunk::removeTile> mapSystemBindListener_EditRemove{world.getSystem<Game::MapSystem>()};

	{
		auto& physSys = world.getSystem<Game::PhysicsSystem>();
		world.getSystem<Game::SpriteSystem>().setup(engine);
		world.getSystem<Game::CameraTrackingSystem>().setup(engine.camera);
		world.getSystem<Game::CharacterSpellSystem>().setup(engine);
		world.getSystem<Game::MapSystem>().setup(engine);
		
		// Player
		world.addComponent<Game::SpriteComponent>(player).texture = engine.textureManager.get("../assets/player.png");
		world.addComponent<Game::PhysicsComponent>(player).body = createPhysicsCircle(player, physSys);
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

	// Callbacks
	glfwSetWindowUserPointer(window, &engine);

	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (action == GLFW_REPEAT) { return; }

		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, true);
		}

		//std::cout << "Keyboard Code: " << scancode << "\tAction: " << action << "\n";

		static_cast<Engine::EngineInstance*>(glfwGetWindowUserPointer(window))
			->inputManager.processInput({{Engine::Input::InputType::KEYBOARD, scancode}, action == GLFW_PRESS});

		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	});

	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x, double y) {
		//std::cout << "Mouse Pos: " << x << ", " << y << "\n";
		static_cast<Engine::EngineInstance*>(glfwGetWindowUserPointer(window))->inputManager.mouseCallback(x, y);
	});

	// TODO: look into "Raw mouse motion" https://www.glfw.org/docs/latest/input_guide.html#raw_mouse_motion
	glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods){
		//std::cout << "Mouse Code: " << button << "\tAction: " << action << "\n";

		static_cast<Engine::EngineInstance*>(glfwGetWindowUserPointer(window))
			->inputManager.processInput({{Engine::Input::InputType::MOUSE, button}, action == GLFW_PRESS});

		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
	});


	// Framebuffer callback
	glfwSetFramebufferSizeCallback(window, framebufferCallback);
	{
		int w;
		int h;
		glfwGetFramebufferSize(window, &w, &h);
		framebufferCallback(window, w, h);
	}

	// ImGui callbacks
	glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
	glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);

	// Procedural test
	mapTest();

	// Main loop
	auto startTime = std::chrono::high_resolution_clock::now();
	auto lastUpdate = startTime;
	while (!glfwWindowShouldClose(window)) {
		// Get the elapsed time in seconds
		auto diff = std::chrono::high_resolution_clock::now() - startTime;
		startTime = std::chrono::high_resolution_clock::now();
		auto dt = std::chrono::duration_cast<
			std::chrono::duration<
				float,
				std::chrono::seconds::period
			>
		>(diff).count();
		
		// Update frame time
		if ((std::chrono::high_resolution_clock::now() - lastUpdate) > std::chrono::seconds{1}) {
			glfwSetWindowTitle(window, std::to_string(dt).c_str());
			lastUpdate = std::chrono::high_resolution_clock::now();
		}

		// Input
		engine.inputManager.update();

		// Rendering
		glClearColor(0.2176f, 0.2176f, 0.2176f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// ECS
		world.run(dt);

		// Physics debug
		#if defined (DEBUG_PHYSICS)
			world.getSystem<Game::PhysicsSystem>().getDebugDraw().draw();
		#endif

		// Draw UI
		doUI(engine, world);

		// GLFW
		glfwSwapBuffers(window);
		glfwPollEvents();

		//std::this_thread::sleep_for(std::chrono::milliseconds{250});
	}

	glDeleteTextures(1, &mapTexture);

	// UI cleanup
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();

	// GLFW cleanup
	glfwDestroyWindow(window);
}


int main(int argc, char* argv[]) {
	std::atexit([](){
		glfwTerminate();
	});

	{ // Position the console
		auto window = GetConsoleWindow();
		SetWindowPos(window, HWND_TOP, 0, 0, 1000, 500, 0);
	}

	run();

	std::cout << "Done." << std::endl;
	return EXIT_SUCCESS;
}
