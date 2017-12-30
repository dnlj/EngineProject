// Windows
#include <Windows.h>

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

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Debug/Debug.hpp>
#include <Engine/Entity.hpp>
#include <Engine/SystemBase.hpp>
#include <Engine/TextureManager.hpp>

namespace {
	constexpr int OPENGL_VERSION_MAJOR = 4;
	constexpr int OPENGL_VERSION_MINOR = 5;

	constexpr char* vertShaderSource = R"(
		#version 450 core
		layout (location = 0) in vec2 vertPos;
		layout (location = 1) in vec2 vertTexCoord;
		
		layout (location = 2) uniform mat4 mvp;
		
		out vec2 fragTexCoord;

		void main() {
			gl_Position = mvp * vec4(vertPos, 0.0, 1.0);
			fragTexCoord = vertTexCoord;
		}
	)";

	constexpr char* fragShaderSource = R"(
		#version 450 core
		in vec2 fragTexCoord;
		out vec4 finalColor;

		// TODO: is this location unique per shader or program? I would assume program? Does it differ by type(attrib vs uniform)?
		layout (location = 6) uniform sampler2D tex;

		void main() {
			finalColor = texture(tex, fragTexCoord);
			//finalColor = vec4(1.0, 0.0, 0.0, 1.0);
		}
	)";

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
}

namespace {
	// TODO: OpenGL cleanup
	class RenderableTest {
		public:
			GLuint vao = 0;
			GLuint vbo = 0;
			GLuint shader = 0;
			GLuint texture = 0;

			// TODO: make this non-static
			~RenderableTest() {
				// TODO: shouldnt do this in deconstructor
				glDeleteVertexArrays(1, &vao);
				glDeleteBuffers(1, &vbo);
				glDeleteProgram(shader);
			}

			void setup(Engine::TextureManager& textureManager) {
				constexpr GLfloat data[] = {
					+0.0f, +0.5f, +0.5, +0.0f,
					-0.5f, -0.5f, +0.0, +1.0f,
					+0.5f, -0.5f, +1.0, +1.0f,
				};

				//texture.load("../assets/test.png", Engine::TextureOptions{Engine::TextureWrap::REPEAT, Engine::TextureFilter::NEAREST, false});
				texture = textureManager.getTexture("../assets/test.png");

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
				shader = glCreateProgram();
				glAttachShader(shader, vertShader);
				glAttachShader(shader, fragShader);
				glLinkProgram(shader);
				glUseProgram(shader); // TODO: This shouldnt need to be here

				{
					GLint status;
					glGetProgramiv(shader, GL_LINK_STATUS, &status);

					if (!status) {
						char buffer[512];
						glGetProgramInfoLog(shader, 512, NULL, buffer);
						std::cout << buffer << std::endl;
					}
				}

				// Shader cleanup
				glDetachShader(shader, vertShader);
				glDetachShader(shader, fragShader);
				glDeleteShader(vertShader);
				glDeleteShader(fragShader);
				// TODO: Error checking when in debug mode
			}
	};
	ENGINE_REGISTER_COMPONENT(RenderableTest);

	class RenderableTestSystem : public Engine::SystemBase {
		public:
			RenderableTestSystem() {
				// TODO: create a better way to do this.
				cbits[Engine::ECS::detail::getComponentID<RenderableTest>()] = true;

				// MVP
				constexpr float scale = 1.0f / 400.0f;
				auto halfWidth = (1280.0f / 2.0f) * scale;
				auto halfHeight = (720.0f / 2.0f) * scale;
				auto projection = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight);
				auto view = glm::mat4{1.0f};
				auto model = glm::mat4{1.0f};
				mvp = projection * view * model;
			}

			void run(float dt) {
				for(auto& ent : entities) {
					const auto& rtest = ent.getComponent<RenderableTest>();
					glBindVertexArray(rtest.vao);
					glUseProgram(rtest.shader);

					// Texture
					// TODO: is this texture stuff stored in VAO?
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, rtest.texture);
					glUniform1i(6, 0);

					// MVP
					glUniformMatrix4fv(2, 1, GL_FALSE, &mvp[0][0]);
					
					// Draw
					glDrawArrays(GL_TRIANGLES, 0, 3);
				}
			}
		private:
			glm::mat4 mvp;
	};
	ENGINE_REGISTER_SYSTEM(RenderableTestSystem);
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

	// ECS Test stuff
	Engine::TextureManager textureManager;
	{
		// TODO: make a create entity with x,y,z components function? to prevent unnessassary calls
		auto& ent = Engine::createEntity();

		// TODO: maybe make an addAndGetComponent function
		ent.addComponent<RenderableTest>();
		ent.getComponent<RenderableTest>().setup(textureManager);
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

		// Rendering
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// ECS
		Engine::ECS::run(dt);

		#if defined(DEBUG)
			Engine::Debug::checkOpenGLErrors();
		#endif

		//std::this_thread::sleep_for(std::chrono::milliseconds{70});

		// GLFW
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

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

	Engine::ECS::init();
	run();

	std::cout << "Done." << std::endl;
	return EXIT_SUCCESS;
}