// STD
#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>

// glLoadGen
#include <glloadgen/gl_core_4_5.h>

// GLFW
#include <GLFW/glfw3.h>

// SOIL
#include <SOIL.h>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Entity.hpp>
#include <Engine/SystemBase.hpp>
#include <Engine/Texture.hpp>

namespace {
	constexpr int OPENGL_VERSION_MAJOR = 4;
	constexpr int OPENGL_VERSION_MINOR = 5;

	constexpr char* vertShaderSource = R"(
		#version 450 core
		layout (location = 0) in vec2 vertPos;
		layout (location = 1) in vec2 vertTexCoord;
		
		out vec2 fragTexCoord;

		void main() {
			gl_Position = vec4(vertPos, 0.0, 1.0);
			fragTexCoord = vertTexCoord;
		}
	)";

	constexpr char* fragShaderSource = R"(
		#version 450 core
		in vec2 fragTexCoord;
		out vec4 finalColor;

		uniform sampler2D tex;

		void main() {
			finalColor = texture(tex, fragTexCoord);
		}
	)";
}

// TODO: Add a tag system that doesnt require storage allocation (it would have to use the same component id things just not craete the arrays)

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
		ENGINE_ERROR("[GLFW] Failed to create window.");
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
		ENGINE_ERROR("[GLFW] Failed to initialize.");
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

	// Image loading
	Engine::Texture texture("../assets/test.png", Engine::TextureOptions{Engine::TextureWrap::REPEAT, Engine::TextureFilter::NEAREST, false});

	// GL stuff
	GLuint vao;
	GLuint vbo;
	GLuint shaderProgram;

	{
		constexpr GLfloat data[] = {
			+0.0f, +0.5f, +0.5, +0.0f,
			-0.5f, -0.5f, +0.0, +1.0f,
			+0.5f, -0.5f, +1.0, +1.0f,
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
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
		
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), reinterpret_cast<const void*>(2 * sizeof(GLfloat)));

		// Vertex shader
		auto vertShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertShader, 1, &vertShaderSource, nullptr);
		glCompileShader(vertShader);

		{
			GLint status;
			glGetShaderiv(vertShader, GL_COMPILE_STATUS, &status);

			if (!status) {
				char buffer[512];
				glGetShaderInfoLog(vertShader, 512, NULL, buffer);
				std::cout << buffer << std::endl;
			}
		}

		// Fragment shader
		auto fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragShader, 1, &fragShaderSource, nullptr);
		glCompileShader(fragShader);

		{
			GLint status;
			glGetShaderiv(fragShader, GL_COMPILE_STATUS, &status);

			if (!status) {
				char buffer[512];
				glGetShaderInfoLog(fragShader, 512, NULL, buffer);
				std::cout << buffer << std::endl;
			}
		}

		// Shader program
		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertShader);
		glAttachShader(shaderProgram, fragShader);
		glLinkProgram(shaderProgram);
		glUseProgram(shaderProgram);
		
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

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture.getID());
		glDrawArrays(GL_TRIANGLES, 0, 3);

		//std::this_thread::sleep_for(std::chrono::milliseconds{70});

		// GLFW
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// OpenGL cleanup
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteProgram(shaderProgram);

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