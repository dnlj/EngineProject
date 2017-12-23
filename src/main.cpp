// STD
#include <algorithm>
#include <iostream>

// glLoadGen
#include <glloadgen/gl_core_4_5.h>

// GLFW
#include <GLFW/glfw3.h>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Entity.hpp>
#include <Engine/SystemBase.hpp>

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


void run() {
	constexpr int OPENGL_VERSION_MAJOR = 4;
	constexpr int OPENGL_VERSION_MINOR = 5;

	// GLFW error callback
	glfwSetErrorCallback([](int error, const char* desc) {
		// TODO: Create a more standard error system
		fprintf(stderr, "[GLFW][Error] %s\n", desc);
	});

	// Initialize GLFW
	if (!glfwInit()) {
		// TODO: Better error
		throw std::runtime_error{"[GLFW] Failed to initialize."};
	}

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
		// TODO: Better error
		throw std::runtime_error{"[GLFW] Failed to create window."};
	}

	glfwMakeContextCurrent(window);
	
	// Enable vsync
	glfwSwapInterval(1);

	// Initialize OpenGL functions
	{
		auto loaded = ogl_LoadFunctions();

		if (loaded == ogl_LOAD_FAILED) {
			// TODO: Better error
			throw std::runtime_error{"[glLoadGen] initialization failed."};
		}

		auto failed = loaded - ogl_LOAD_SUCCEEDED;
		if (failed > 0) {
			// TODO: Better error
			throw std::runtime_error{"[glLoadGen] Failed to load " + std::to_string(failed) + " functions."};
		}


		if (!ogl_IsVersionGEQ(OPENGL_VERSION_MAJOR, OPENGL_VERSION_MINOR)) {
			throw std::runtime_error("[glLoadGen] OpenGL version " + std::to_string(OPENGL_VERSION_MAJOR) + "." + std::to_string(OPENGL_VERSION_MINOR) + " is not available.");
		}
	}

	// Key callbacks
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, true);
		}
	});

	// Main loop
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
}


int main(int argc, char* argv[]) {
	Engine::ECS::init();
	run();

	std::cout << "Done." << std::endl;
	return 0;
}