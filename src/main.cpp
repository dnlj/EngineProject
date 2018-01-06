// Windows
#include <Windows.h>

// STD
#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>

// glLoadGen
#include <glloadgen/gl_core_4_5.h>

// GLFW
#include <GLFW/glfw3.h>

// SOIL
#include <SOIL.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Box2D
#include <Box2D/Box2D.h>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Debug/Debug.hpp>
#include <Engine/Entity.hpp>
#include <Engine/SystemBase.hpp>
#include <Engine/TextureManager.hpp>
#include <Engine/Utility/Utility.hpp>

namespace {
	constexpr int OPENGL_VERSION_MAJOR = 4;
	constexpr int OPENGL_VERSION_MINOR = 5;
	GLFWwindow* window = nullptr; // TODO: need to add a way to pass data to systems

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
	class RenderableTest {
		public:
			GLuint vao = 0;
			GLuint vbo = 0;
			GLuint shader = 0;
			GLuint texture = 0;
			glm::vec2 position{};

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
				{
					const auto source = Engine::Utility::readFile("shaders/vertex.glsl");
					const auto cstr = source.c_str();
					glShaderSource(vertShader, 1, &cstr, nullptr);
				}
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
				{
					const auto source = Engine::Utility::readFile("shaders/fragment.glsl");
					const auto cstr = source.c_str();
					glShaderSource(fragShader, 1, &cstr, nullptr);
				}
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
			}
	};
	ENGINE_REGISTER_COMPONENT(RenderableTest);

	class Component2 {};
	ENGINE_REGISTER_COMPONENT(Component2);

	glm::mat4 projection; // TODO: Make this not global
	glm::mat4 view; // TODO: Make this not global
	class RenderableTestMovement;
	class RenderableTestSystem : public Engine::SystemBase {
		public:
			RenderableTestSystem() {
				cbits = Engine::ECS::getBitsetForComponents<RenderableTest>();

				// MVP
				constexpr float scale = 1.0f / 400.0f;
				auto halfWidth = (1280.0f / 2.0f) * scale;
				auto halfHeight = (720.0f / 2.0f) * scale;
				projection = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight);
				view = glm::mat4{1.0f};
			}

			void run(float dt) {
				std::cout << " - Draw run\n";
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
					{
						auto model = glm::translate(glm::mat4{1}, glm::vec3{rtest.position, 0.0f});
						glm::mat4 mvp = projection * view * model;
						glUniformMatrix4fv(2, 1, GL_FALSE, &mvp[0][0]);
					}
					
					// Draw
					glDrawArrays(GL_TRIANGLES, 0, 3);
				}
			}
		private:
	};
	ENGINE_REGISTER_SYSTEM(RenderableTestSystem);

	class RenderableTestMovement : public Engine::SystemBase {
		public:
			RenderableTestMovement() {
				cbits = Engine::ECS::getBitsetForComponents<RenderableTest>();
				priorityBefore = Engine::ECS::getBitsetForSystems<RenderableTestSystem>();
			}

			void run(float dt) {
				std::cout << " - Move run\n";
				constexpr float speed = 1.0f;
				for (auto& ent : entities) {
					auto& rtest = ent.getComponent<RenderableTest>();
					
					if (glfwGetKey(window, GLFW_KEY_W)) {
						rtest.position.y += speed * dt;
					}

					if (glfwGetKey(window, GLFW_KEY_S)) {
						rtest.position.y -= speed * dt;
					}

					if (glfwGetKey(window, GLFW_KEY_A)) {
						rtest.position.x -= speed * dt;
					}

					if (glfwGetKey(window, GLFW_KEY_D)) {
						rtest.position.x += speed * dt;
					}
				}
			}
	};
	ENGINE_REGISTER_SYSTEM(RenderableTestMovement);

	class RenderableTestSystem3 : public Engine::SystemBase {
		public:
			RenderableTestSystem3() {
				priorityBefore = Engine::ECS::getBitsetForSystems<
					RenderableTestSystem,
					RenderableTestMovement
				>();
			}

			void run(float dt) {
				std::cout << " - System3 run\n";
			}
	};
	ENGINE_REGISTER_SYSTEM(RenderableTestSystem3);
}

namespace {
	class DebugDraw : public b2Draw {
		private:
			struct Vertex {
				public:
				b2Vec2 pos;
				b2Color color;
			};

		public:
			DebugDraw() {
				glGenVertexArrays(1, &vao);
				glBindVertexArray(vao);

				glGenBuffers(1, &vbo);
				glBindBuffer(GL_ARRAY_BUFFER, vbo);
				glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vertex), vertexData.data(), GL_DYNAMIC_DRAW);

				// Vertex attributes
				constexpr auto stride = sizeof(b2Vec2) + sizeof(b2Color);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, 0);

				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(sizeof(b2Vec2)));
				
				// Vertex shader
				auto vertShader = glCreateShader(GL_VERTEX_SHADER);
				{
					const auto source = Engine::Utility::readFile("shaders/box2d_debug.vert");
					const auto cstr = source.c_str();
					glShaderSource(vertShader, 1, &cstr, nullptr);
				}
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
				{
					const auto source = Engine::Utility::readFile("shaders/box2d_debug.frag");
					const auto cstr = source.c_str();
					glShaderSource(fragShader, 1, &cstr, nullptr);
				}
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
			}

			~DebugDraw() {
				glDeleteVertexArrays(1, &vao);
				glDeleteBuffers(1, &vbo);
				glDeleteProgram(shader);
			}

			void reset() {
				vertexCount = 0;
			}

			void draw() {
				glBindVertexArray(vao);

				glBindBuffer(GL_ARRAY_BUFFER, vbo);
				glBufferSubData(GL_ARRAY_BUFFER, 0, vertexCount * sizeof(Vertex), vertexData.data());

				glUseProgram(shader);

				glm::mat4 pv = projection * view;
				glUniformMatrix4fv(2, 1, GL_FALSE, &pv[0][0]);

				glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexData.size()));
			}
			
			virtual void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override {
				for (int32 i = 0; i < vertexCount - 1; ++i) {
					DrawSegmentInside(vertices[i], vertices[i + 1], color);
				}

				DrawSegmentInside(vertices[vertexCount - 1], vertices[0], color);
			}

			virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override {
				b2Color fillColor{FILL_COLOR_MULT * color.r, FILL_COLOR_MULT * color.g, FILL_COLOR_MULT * color.b, FILL_COLOR_MULT * color.a};

				for (int32 i = 1; i < vertexCount - 1; ++i) {
					addVertex(Vertex{vertices[0], fillColor});
					addVertex(Vertex{vertices[i], fillColor});
					addVertex(Vertex{vertices[i + 1], fillColor});
				}

				DrawPolygon(vertices, vertexCount, color);
			}

			virtual void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) override {
				const auto vertices = getCircleVertices(center, radius);
				DrawPolygon(vertices.data(), static_cast<int>(vertices.size()), color);
			}

			virtual void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) override {
				const auto vertices = getCircleVertices(center, radius);
				DrawSolidPolygon(vertices.data(), static_cast<int>(vertices.size()), color);
				DrawSegment(center, center + radius * axis, color);
			}

			virtual void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override {
				// Get a scaled normal vector
				b2Vec2 normal{p2.y - p1.y, -p2.x + p1.x};
				normal.Normalize();
				normal *= LINE_SIZE / 2.0f;
				
				// Compute points
				auto v1 = p1 - normal;
				auto v2 = p1 + normal;
				auto v3 = p2 + normal;
				auto v4 = p2 - normal;

				// Add the data
				addVertex(Vertex{v1, color});
				addVertex(Vertex{v2, color});
				addVertex(Vertex{v3, color});

				addVertex(Vertex{v3, color});
				addVertex(Vertex{v4, color});
				addVertex(Vertex{v1, color});
			}

			void DrawSegmentInside(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) {
				// Get a scaled normal vector
				b2Vec2 normal{p2.y - p1.y, -p2.x + p1.x};
				normal.Normalize();
				normal *= LINE_SIZE;

				// Compute points
				auto v1 = p1 - normal;
				auto v2 = p1;
				auto v3 = p2;
				auto v4 = p2 - normal;

				// Add the data
				addVertex(Vertex{v1, color});
				addVertex(Vertex{v2, color});
				addVertex(Vertex{v3, color});
				
				addVertex(Vertex{v3, color});
				addVertex(Vertex{v4, color});
				addVertex(Vertex{v1, color});
			}

			virtual void DrawTransform(const b2Transform& xf) override {
				DrawSegment(xf.p, xf.p + AXIS_SIZE * xf.q.GetXAxis(), b2Color{1.0f, 0.0f, 0.0f});
				DrawSegment(xf.p, xf.p + AXIS_SIZE * xf.q.GetYAxis(), b2Color{0.0f, 1.0f, 0.0f});
			}

			virtual void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color) override {
				std::cout << "DrawPoint\n";
			}

		private:
			static constexpr float LINE_SIZE = 0.008f;
			static constexpr float AXIS_SIZE = 0.1f;
			static constexpr float FILL_COLOR_MULT = 0.5f;

			std::array<Vertex, 512> vertexData;
			size_t vertexCount = 0;
			GLuint vao;
			GLuint vbo;
			GLuint shader;

			void addVertex(Vertex vertex) {
				if (vertexCount == vertexData.size()) {
					ENGINE_WARN("To many debug vertices. Increase MAX_VERTICES");
				} else {
					vertexData[vertexCount] = vertex;
					++vertexCount;
				}
			}

			std::vector<b2Vec2> getCircleVertices(const b2Vec2& center, float32 radius) const {
				// TODO: Redo this formula to calculate the number need to have a certain angle between edge segments.
				const unsigned int vertCount = 16 + static_cast<unsigned int>(std::max(0.0f, (radius - 0.2f) * 5.0f));
				const float angleInc = glm::two_pi<float>() / vertCount;

				std::vector<b2Vec2> vertices{vertCount};

				for (unsigned int i = 0; i < vertCount; ++i) {
					vertices[i].x = cos(i * angleInc) * radius;
					vertices[i].y = sin(i * angleInc) * radius;
					vertices[i] += center;
				}

				return vertices;
			}
	};
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
	window = createWindow();
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

	// Box2D testing
	b2World world{b2Vec2{0.0f, -0.0f}};
	b2Body* body;
	DebugDraw debugDraw;
	{
		debugDraw.SetFlags(b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_aabbBit | b2Draw::e_pairBit | b2Draw::e_centerOfMassBit);
		world.SetDebugDraw(&debugDraw);

		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;

		body = world.CreateBody(&bodyDef);

		//b2PolygonShape boxShape;
		//boxShape.SetAsBox(0.5f, 0.5f);
		b2CircleShape boxShape;
		boxShape.m_radius = 0.5f;

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &boxShape;
		fixtureDef.density = 1.0f;
		body->CreateFixture(&fixtureDef);
	}

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
		std::cout << "== New run ==\n";

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

		// Box2D
		world.Step(dt, 8, 3);
		debugDraw.reset();
		world.DrawDebugData();

		// ECS
		Engine::ECS::run(dt);

		#if defined(DEBUG)
			Engine::Debug::checkOpenGLErrors();
		#endif

		//std::this_thread::sleep_for(std::chrono::milliseconds{70});

		// Box2D debug draw
		debugDraw.draw();

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