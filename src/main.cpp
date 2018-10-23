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

// Game
#include <Game/Common.hpp>
#include <Game/SpriteComponent.hpp>
#include <Game/PhysicsComponent.hpp>
#include <Game/imgui_impl_glfw_gl3.hpp>


namespace {
	constexpr int OPENGL_VERSION_MAJOR = 4;
	constexpr int OPENGL_VERSION_MINOR = 5;

	void editorUI();

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

	void doUI() {
		static bool showWindow = true;

		ImGui_ImplGlfwGL3_NewFrame();

		ImGui::ShowDemoWindow(&showWindow);
		editorUI();

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

namespace {
	class Tile {
		public:
			const int id;
	};

	class Chunk {
		public:
			constexpr static Tile AIR{0};
			constexpr static Tile DIRT{1};

		public:
			constexpr static int width = 16;
			constexpr static int height = width;
			constexpr static auto halfSize = 1.0f/8.0f; // TODO: change to tileSize. halfSize is only used once

		private:
			int data[width][height] = {
				{3, 0, 0, 0, 0, 0, 0, 3},
				{0, 2, 0, 0, 0, 0, 2, 0},
				{0, 0, 2, 2, 2, 2, 0, 0},
				{0, 0, 2, 1, 1, 2, 0, 0},
				{0, 0, 2, 0, 1, 2, 0, 0},
				{0, 0, 2, 2, 2, 2, 0, 0},
				{0, 0, 0, 0, 0, 0, 2, 0},
				{4, 0, 0, 0, 0, 0, 0, 3},
			};

			b2Body* body = nullptr; // TODO: Cleanup
			Engine::ECS::Entity ent;

		public:
			void setup(Game::World& world, glm::vec2 pos) {
				ent = world.createEntity(true);
				generate(world.getSystem<Game::PhysicsSystem>());
				body->SetTransform(b2Vec2{pos.x, pos.y}, 0.0f);
			}

			void update(Engine::EngineInstance& engine, Game::World& world) {
				auto& im = engine.inputManager;
				const auto& cam = engine.camera;

				// TODO: Reduce duplicate code

				if (im.isPressed("edit_place")) {
					auto mpos = cam.screenToWorld(im.getMousePosition());
					auto pos = body->GetPosition();

					auto offset = mpos - glm::vec2{pos.x, pos.y};
					offset /= (halfSize * 2);

					// TODO: this only works for pos = 0,0
					if (offset.x < 0 || offset.x > width) { return; }
					if (offset.y < 0 || offset.y > height) { return; }

					std::cout << "Chunk - x: " << offset.x << " y: " << offset.y << "\n";

					auto& tile = data[static_cast<int>(offset.x)][static_cast<int>(offset.y)];
					if (tile != 0) { return; }
					tile = 1;
					generate(world.getSystem<Game::PhysicsSystem>());
				}

				if (im.isPressed("edit_remove")) {
					auto mpos = cam.screenToWorld(im.getMousePosition());
					auto pos = body->GetPosition();

					auto offset = mpos - glm::vec2{pos.x, pos.y};
					offset /= (halfSize * 2);

					if (offset.x < 0 || offset.x > width) { return; }
					if (offset.y < 0 || offset.y > height) { return; }

					auto& tile = data[static_cast<int>(offset.x)][static_cast<int>(offset.y)];
					if (tile == 0) { return; }
					tile = 0;
					generate(world.getSystem<Game::PhysicsSystem>());
				}
			}

			void generate(Game::PhysicsSystem& physSys) {
				// TODO: Look into edge and chain shapes
				b2Vec2 oldPos = b2Vec2_zero;

				if (body != nullptr) {
					oldPos = body->GetPosition();
					physSys.destroyBody(body);
				}

				b2BodyDef bodyDef;
				bodyDef.type = b2_staticBody;
				bodyDef.awake = false;
				bodyDef.fixedRotation = true;

				b2PolygonShape shape;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &shape;

				body = physSys.createBody(ent, bodyDef);

				constexpr float size = halfSize * 2.0f;
				bool used[width][height]{};


				auto expand = [&](const int ix, const int iy) {
					int w = 0;
					int h = 0;
					bool expandWidth = true;
					bool expandHeight = true;

					while (expandWidth || expandHeight) {
						if (expandWidth) {
							const auto limit = std::min(iy + h, height);
							for (int y = iy; y < limit; ++y) {
								if (used[ix + w][y] || data[ix + w][y] == AIR.id) {
									if (w == 0) { return; }
									expandWidth = false;
									break;
								}
							}

							if (expandWidth) {
								for (int y = iy; y < limit; ++y) {
									used[ix + w][y] = true;
								}

								++w;

								if (ix + w == width) {
									expandWidth = false;
								}
							}
						}

						if (expandHeight) {
							const auto limit = std::min(ix + w, width);
							for (int x = ix; x < limit; ++x) {
								if (used[x][iy + h] || data[x][iy + h] == AIR.id) {
									if (h == 0) { return; }
									expandHeight = false;
									break;
								}
							}

							if (expandHeight) {
								for (int x = ix; x < limit; ++x) {
									used[x][iy + h] = true;
								}

								++h;

								if (iy + h == height) {
									expandHeight = false;
								}
							}
						}
					}

					shape.SetAsBox(
						halfSize * w,
						halfSize * h,
						b2Vec2(
							(ix + w/2.0f) * size,
							(iy + h/2.0f) * size
						),
						0.0f
					);

					body->CreateFixture(&fixtureDef);
				};

				for (int x = 0; x < width; ++x) {
					for (int y = 0; y < height; ++y) {
						expand(x, y);
					}
				}

				body->SetTransform(oldPos, 0.0f);
			}
	};

	class Map {
		private:
			constexpr static int chunkCountX = 4;
			constexpr static int chunkCountY = chunkCountX;

			Chunk chunks[chunkCountX][chunkCountY]{};

		public:
			void setup(Game::World& world) {
				for (int y = 0; y < chunkCountY; ++y) {
					for (int x = 0; x < chunkCountX; ++x) {
						chunks[x][y].setup(world, glm::vec2{
							x * Chunk::width * Chunk::halfSize * 2.0f,
							y * Chunk::height * Chunk::halfSize * 2.0f
						});
					}
				}
			}

			void update(Engine::EngineInstance& engine, Game::World& world) {
				auto& im = engine.inputManager;
				const auto& cam = engine.camera;

				// TODO: Reduce duplicate code

				if (im.isPressed("edit_place")) {
					// TODO: Rework this logic. its trash.
					auto mpos = cam.screenToWorld(im.getMousePosition());
					constexpr auto pos = glm::vec2{0, 0};
					auto bounds = pos + glm::vec2{chunkCountX * Chunk::width, chunkCountY * Chunk::height};

					auto offset = mpos - pos;
					offset /= (Chunk::halfSize * 2);

					// TODO: this only works if pos = 0,0
					if (offset.x < 0 || offset.x >= bounds.x) { return; }
					if (offset.y < 0 || offset.y >= bounds.y) { return; }

					const auto ix = static_cast<int>(offset.x / Chunk::width);
					const auto iy = static_cast<int>(offset.y / Chunk::height);

					std::cout << "Map - x: " << ix << " y: " << iy << "\n";

					// TODO: passthorugh offset
					chunks[ix][iy].update(engine, world);
				}
			}
	};

	void editorUI() {
		bool open = true;
		ImGui::Begin("Editor UI", &open, ImGuiWindowFlags_MenuBar);

		ImGui::Text("test");

		ImGui::End();
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
	Map map;

	{
		auto& physSys = world.getSystem<Game::PhysicsSystem>();
		world.getSystem<Game::SpriteSystem>().setup(engine.camera);
		world.getSystem<Game::CameraTrackingSystem>().setup(engine.camera);
		world.getSystem<Game::CharacterSpellSystem>().setup(engine);

		// Map
		map.setup(world);
		
		// Player
		auto player = world.createEntity();
		world.addComponent<Game::SpriteComponent>(player).texture = engine.textureManager.getTexture("../assets/player.png");
		world.addComponent<Game::PhysicsComponent>(player).body = createPhysicsCircle(player, physSys);
		world.addComponent<Game::CharacterMovementComponent>(player);
		world.addComponent<Game::CharacterSpellComponent>(player);
		world.addComponent<Game::InputComponent>(player).inputManager = &engine.inputManager;

		world.getSystem<Game::CameraTrackingSystem>().focus = player;
		
		//// Other
		//auto other = world.createEntity();
		////world.addComponent<Game::RenderComponent>(other).setup(engine.textureManager);
		//world.addComponent<Game::PhysicsComponent>(other).body = createPhysicsCircle(physSys.getPhysicsWorld());
		//
		//// Level
		//auto level = world.createEntity();
		//world.addComponent<Game::PhysicsComponent>(level).body = createPhysicsLevel(physSys.getPhysicsWorld());

		//constexpr int half = 4;
		//constexpr float scale = 0.26f;
		//const b2Vec2 offset{scale * (half + 1), 0.0f};
		//
		//for (int x = -half; x < half; ++x) {
		//	for (int y = -half; y < half; ++y) {
		//		auto ent = world.createEntity();
		//
		//		world.addComponent<Game::SpriteComponent>(ent).texture
		//			= engine.textureManager.getTexture("../assets/test.png");
		//
		//		world.addComponent<Game::PhysicsComponent>(ent).body
		//			= createPhysicsSquare(ent, physSys, offset + b2Vec2(scale * x, scale * y));
		//	}
		//}
	}

	// Binds
	engine.inputManager.bindkey(17, "MoveUp");
	engine.inputManager.bindkey(31, "MoveDown");
	engine.inputManager.bindkey(30, "MoveLeft");
	engine.inputManager.bindkey(32, "MoveRight");
	engine.inputManager.bindkey(57, "Spell_1");
	engine.inputManager.bindMouseButton(0, "edit_place");
	engine.inputManager.bindMouseButton(1, "edit_remove");

	// Callbacks
	glfwSetWindowUserPointer(window, &engine);

	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, true);
		}

		static_cast<Engine::EngineInstance*>(glfwGetWindowUserPointer(window))->inputManager.keyCallback(scancode, action);

		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	});

	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x, double y) {
		static_cast<Engine::EngineInstance*>(glfwGetWindowUserPointer(window))->inputManager.mouseCallback(x, y);
	});

	glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods){
		static_cast<Engine::EngineInstance*>(glfwGetWindowUserPointer(window))->inputManager.mouseCallback(button, action);
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

		// Editor
		map.update(engine, world);

		// ECS
		world.run(dt);
		engine.inputManager.update();

		// Physics debug
		#if defined (DEBUG_PHYSICS)
			world.getSystem<Game::PhysicsSystem>().getDebugDraw().draw(engine.camera.getProjection(), engine.camera.getView());
		#endif

		//std::this_thread::sleep_for(std::chrono::milliseconds{70});

		// Draw UI
		doUI();

		// GLFW
		glfwSwapBuffers(window);
		glfwPollEvents();
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
