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

	void editorUI(Engine::EngineInstance& engine, Game::World& world) {
		bool open = true;
		ImGui::Begin("Editor UI", &open, ImGuiWindowFlags_MenuBar);

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
		constexpr int width = 1280;
		constexpr int height = 720;
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
	engine.inputManager.addInputBindMapping(Engine::InputSequence{
		Engine::Input{Engine::InputType::KEYBOARD, 29}, Engine::Input{Engine::InputType::KEYBOARD, 46} // CTRL + C
	}, "Spell_1");
	engine.inputManager.addInputBindMapping(Engine::InputSequence{
		Engine::Input{Engine::InputType::KEYBOARD, 29}, // CTRL
		Engine::Input{Engine::InputType::KEYBOARD, 56}, // ALT
		Engine::Input{Engine::InputType::KEYBOARD, 16}, // Q
	}, "Spell_1");
	engine.inputManager.addInputBindMapping(Engine::InputSequence{Engine::Input{Engine::InputType::KEYBOARD, 57}}, "Spell_1");
	engine.inputManager.addInputBindMapping(Engine::InputSequence{Engine::Input{Engine::InputType::KEYBOARD, 17}}, "MoveUp");
	engine.inputManager.addInputBindMapping(Engine::InputSequence{Engine::Input{Engine::InputType::KEYBOARD, 31}}, "MoveDown");
	engine.inputManager.addInputBindMapping(Engine::InputSequence{Engine::Input{Engine::InputType::KEYBOARD, 30}}, "MoveLeft");
	engine.inputManager.addInputBindMapping(Engine::InputSequence{Engine::Input{Engine::InputType::KEYBOARD, 32}}, "MoveRight");
	engine.inputManager.addInputBindMapping(Engine::InputSequence{Engine::Input{Engine::InputType::MOUSE, 0}}, "EditPlace");
	engine.inputManager.addInputBindMapping(Engine::InputSequence{Engine::Input{Engine::InputType::MOUSE, 1}}, "EditRemove");

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
			->inputManager.processInput({{Engine::InputType::KEYBOARD, scancode}, action == GLFW_PRESS});

		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	});

	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x, double y) {
		static_cast<Engine::EngineInstance*>(glfwGetWindowUserPointer(window))->inputManager.mouseCallback(x, y);
	});

	// TODO: look into "Raw mouse motion" https://www.glfw.org/docs/latest/input_guide.html#raw_mouse_motion
	glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods){
		//std::cout << "Mouse Code: " << button << "\tAction: " << action << "\n";

		static_cast<Engine::EngineInstance*>(glfwGetWindowUserPointer(window))
			->inputManager.processInput({{Engine::InputType::MOUSE, button}, action == GLFW_PRESS});

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

		// Rendering
		glClearColor(0.2176f, 0.2176f, 0.2176f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// ECS
		world.run(dt);
		engine.inputManager.update();

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
