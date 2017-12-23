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

	constexpr char* vertShaderSource = R"(
		#version 450 core
		layout (location = 0) in vec2 vert;

		void main() {
			gl_Positino = vec4(vert, 0.0, 1.0);
		}
	)";

	constexpr char* fragShaderSource = R"(
		#version 450 core
		out vec4 finalColor;		

		void main() {
			finalColor = vec4(1.0, 0.0, 0.0, 1.0);
		}
	)";
}

// TODO: Add a tag system that doesnt require storage allocation (it would have to use the same component id things just not craete the arrays)

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

	// GL stuff
	GLuint vao;
	GLuint vbo;

	{
		constexpr GLfloat data[] = {
			+0.0f, +0.5f,
			-0.5f, -0.5f,
			+0.5f, -0.5f,
		};

		// VAO
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		// VBO
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(data), &data, GL_STATIC_DRAW);

		// Vertex attributes
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);

		// Vertex shader
		auto vertShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertShader, 1, &vertShaderSource, nullptr);
		glCompileShader(vertShader);

		// Fragment shader
		auto fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragShader, 1, &fragShaderSource, nullptr);
		glCompileShader(fragShader);

		// Shader program
		auto shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertShader);
		glAttachShader(shaderProgram, fragShader);
		glLinkProgram(shaderProgram);
		
		// Shader cleanup
		glDetachShader(shaderProgram, vertShader);
		glDetachShader(shaderProgram, fragShader);
		glDeleteShader(vertShader);
		glDeleteShader(fragShader);

		// TODO: Error checking when in debug mode
	}

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

		// ECS
		Engine::ECS::run(dt);

		// Rendering
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		// GLFW
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// OpenGL cleanup
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

	// GLFW cleanup
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