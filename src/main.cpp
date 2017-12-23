// STD
#include <algorithm>
#include <iostream>
#include <chrono>

// glLoadGen
#include <glloadgen/gl_core_4_5.h>

// GLFW
#include <GLFW/glfw3.h>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Entity.hpp>
#include <Engine/SystemBase.hpp>

namespace {
	constexpr int OPENGL_VERSION_MAJOR = 4;
	constexpr int OPENGL_VERSION_MINOR = 5;
}

// TODO: Add a tag system that doesnt require storage allocation (it would have to use the same component id things just not craete the arrays)

namespace {
	class ComponentA {
		public:
			int a = -1;
			int b = -2;
	};

	class ComponentB {
		public:
			float c = -3.0f;
			float d = -4.0f;
	};

	class ComponentC {
		public:
			double e = -5.0;
			double f = -6.0;
	};
}

ENGINE_REGISTER_COMPONENT(ComponentA);
ENGINE_REGISTER_COMPONENT(ComponentB);
ENGINE_REGISTER_COMPONENT(ComponentC);


namespace {
	class SystemA : public Engine::SystemBase {
		public:
			SystemA() {
				cbits[Engine::ECS::detail::getComponentID<ComponentA>()] = true;
				cbits[Engine::ECS::detail::getComponentID<ComponentB>()] = true;
			}

			void run(float dt) {
				std::cout << "A Run: " << dt << std::endl;
				
				for (auto& ent : entities) {
					std::cout << "A process: " << ent.getID() << "\n";
				}
			};
	};

	class SystemB : public Engine::SystemBase {
		public:
			void onEntityCreated(Engine::Entity ent) {
				std::cout << "B create: " << ent << std::endl;
			}

			void onComponentAdded(Engine::Entity ent, Engine::ECS::ComponentID cid) {
				std::cout << "B add: " << ent << " " << cid << std::endl;
			}

			void onComponentRemoved(Engine::Entity ent, Engine::ECS::ComponentID cid) {
				std::cout << "B remove: " << ent << " " << cid << std::endl;
			}

			void onEntityDestroyed(Engine::Entity ent) {
				std::cout << "B destroy: " << ent << std::endl;
			}

			void run(float dt) {
				std::cout << "B Run: " << dt << std::endl;
			};
	};
}

ENGINE_REGISTER_SYSTEM(SystemA);
ENGINE_REGISTER_SYSTEM(SystemB);

// TODO: Create a proper logging/warning/error library
namespace Log {
	void log(std::string_view msg) {
		std::clog << "[LOG] " << msg << "\n";
	}

	void warn(std::string_view msg) {
		std::cerr << "[WARN] " << msg << "\n";
	}

	void error(std::string_view msg) {
		std::cerr << "[ERROR] " << msg << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

void initializeOpenGL() {
	auto loaded = ogl_LoadFunctions();

	if (loaded == ogl_LOAD_FAILED) {
		Log::error("[glLoadGen] initialization failed.");
	}

	auto failed = loaded - ogl_LOAD_SUCCEEDED;
	if (failed > 0) {
		Log::error("[glLoadGen] Failed to load " + std::to_string(failed) + " functions.");
	}


	if (!ogl_IsVersionGEQ(OPENGL_VERSION_MAJOR, OPENGL_VERSION_MINOR)) {
		Log::error("[glLoadGen] OpenGL version " + std::to_string(OPENGL_VERSION_MAJOR) + "." + std::to_string(OPENGL_VERSION_MINOR) + " is not available.");
	}
}

GLFWwindow* createWindow() {
	// GLFW hints
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_DECORATED, GL_TRUE);

	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_DEPTH_BITS, 32);

	// Create a window
	auto window = glfwCreateWindow(1280, 720, "Window Title", nullptr, nullptr);

	if (!window) {
		Log::error("[GLFW] Failed to create window.");
	}

	return window;
}

void run() {
	// GLFW error callback
	glfwSetErrorCallback([](int error, const char* desc) {
		// TODO: Create a more standard error system
		fprintf(stderr, "[GLFW] %s\n", desc);
	});

	// Initialize GLFW
	if (!glfwInit()) {
		Log::error("[GLFW] Failed to initialize.");
	}

	// Create a window
	auto window = createWindow();
	glfwMakeContextCurrent(window);
	
	// Enable vsync
	glfwSwapInterval(1);

	// Initialize OpenGL functions
	initializeOpenGL();

	// Key callbacks
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, true);
		}
	});

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

		// Other
		Engine::ECS::run(dt);
		glfwPollEvents();
		glfwSwapBuffers(window);

	}

	glfwDestroyWindow(window);
}


int main(int argc, char* argv[]) {
	std::atexit([](){
		glfwTerminate();
	});

	Engine::ECS::init();
	run();

	std::cout << "Done." << std::endl;
	return EXIT_SUCCESS;
}