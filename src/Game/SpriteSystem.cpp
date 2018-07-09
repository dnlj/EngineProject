// GLM
#include <glm/gtc/matrix_transform.hpp>

// Engine
#include <Engine/Utility/Utility.hpp>

// Game
#include <Game/SpriteSystem.hpp>

namespace Game {
	SpriteSystem::SpriteSystem(World& world) : SystemBase{world} {
		cbits = world.getBitsetForComponents<Game::SpriteComponent, Game::PhysicsComponent>();
		// TODO: Remove render system
		priorityAfter = world.getBitsetForSystems<Game::PhysicsSystem, Game::RenderSystem>();
	}

	void SpriteSystem::setup(const Engine::Camera& camera) {
		this->camera = &camera;

		// TODO: DSA?
		// TODO: Split into multiple functions?

		{// Shader programs
			// TODO: SSO?
			// Vertex shader
			auto vertShader = glCreateShader(GL_VERTEX_SHADER);
			{
				const auto source = Engine::Utility::readFile("./shaders/sprite.vert");
				const auto cstr = source.c_str();
				glShaderSource(vertShader, 1, &cstr, nullptr);
			}
			glCompileShader(vertShader);

			// Fragment shader
			auto fragShader = glCreateShader(GL_FRAGMENT_SHADER);
			{
				const auto source = Engine::Utility::readFile("./shaders/sprite.frag");
				const auto cstr = source.c_str();
				glShaderSource(fragShader, 1, &cstr, nullptr);
			}
			glCompileShader(fragShader);

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

		{ // Vertex array
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);
		}

		{ // Vertex buffer
			Vertex data[] = {
				Vertex{glm::vec2{-0.5f, +0.5f},   glm::vec2{+0.0f, +1.0f}},
				Vertex{glm::vec2{-0.5f, -0.5f},   glm::vec2{+0.0f, +0.0f}},
				Vertex{glm::vec2{+0.5f, -0.5f},   glm::vec2{+1.0f, +0.0f}},
				Vertex{glm::vec2{+0.5f, +0.5f},   glm::vec2{+1.0f, +1.0f}},
			};

			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(data), &data, GL_STATIC_DRAW);
		}

		{ // Instance vertex buffer
			glGenBuffers(1, &ivbo);
			glBindBuffer(GL_ARRAY_BUFFER, ivbo);
			glBufferData(GL_ARRAY_BUFFER, MAX_SPRITES * sizeof(InstanceData), nullptr, GL_DYNAMIC_DRAW);
			instanceData.reserve(MAX_SPRITES);
		}

		{ // Element buffer
			GLubyte data[] = {
				0, 1, 2,
				2, 3, 0,
			};

			glGenBuffers(1, &ebo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(data), &data, GL_STATIC_DRAW);
		}

		{ // Vertex attributes
			// TODO: Look into https://www.khronos.org/opengl/wiki/Vertex_Specification#Separate_attribute_format
			constexpr GLsizei vertexStride = sizeof(Vertex);
			constexpr GLsizei instanceStride = sizeof(InstanceData);

			// Vertex attributes
			glBindBuffer(GL_ARRAY_BUFFER, vbo);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, vertexStride, reinterpret_cast<const void*>(offsetof(Vertex, position)));
			
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vertexStride, reinterpret_cast<const void*>(offsetof(Vertex, texCoord)));

			// Instance attributes
			glBindBuffer(GL_ARRAY_BUFFER, ivbo);

			glEnableVertexAttribArray(2);
			glEnableVertexAttribArray(3);
			glEnableVertexAttribArray(4);
			glEnableVertexAttribArray(5);
			glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, instanceStride, reinterpret_cast<const void*>(offsetof(InstanceData, mvp) +  0 * sizeof(GLfloat)));
			glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, instanceStride, reinterpret_cast<const void*>(offsetof(InstanceData, mvp) +  4 * sizeof(GLfloat)));
			glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, instanceStride, reinterpret_cast<const void*>(offsetof(InstanceData, mvp) +  8 * sizeof(GLfloat)));
			glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, instanceStride, reinterpret_cast<const void*>(offsetof(InstanceData, mvp) + 12 * sizeof(GLfloat)));
			glVertexAttribDivisor(2, 1);
			glVertexAttribDivisor(3, 1);
			glVertexAttribDivisor(4, 1);
			glVertexAttribDivisor(5, 1);
		}
	}

	void SpriteSystem::run(float dt) {
		if (entities.empty()) { return; }

		glBindVertexArray(vao);
		glUseProgram(shader);

		// TODO: Look into SSBO, UBO, Buffer Texture, Vertex Attribute Divisor for mvp
		// TODO: Look into using a geometry shader then (in a ubo?) we only would need to pass: width, height, mvp
		// TODO: Although if we have static geometry (such as the level) it would probably be faster to use a VBO instead of a geometry shader
		// TODO: Look into array textures (GL_TEXTURE_2D_ARRAY)

		// Sort by texture
		auto entitiesByTexture = entities;
		std::sort(entitiesByTexture.begin(), entitiesByTexture.end(), [this](Engine::ECS::EntityID a, Engine::ECS::EntityID b) {
			return world.getComponent<Game::SpriteComponent>(a).texture < world.getComponent<Game::SpriteComponent>(b).texture;
		});

		// Populate data
		instanceData.clear();
		spriteGroups.clear();
		spriteGroups.emplace_back();

		for (auto& eid : entitiesByTexture) {
			const auto& spriteComp = world.getComponent<Game::SpriteComponent>(eid);
			const auto& physComp = world.getComponent<Game::PhysicsComponent>(eid);
			const auto& transform = physComp.body->GetTransform();

			// Set camera uniform
			auto model = glm::translate(glm::mat4{1.0f}, glm::vec3{transform.p.x, transform.p.y, 0.0f}) * glm::scale(glm::mat4{1.0f}, glm::vec3{0.25f});
			glm::mat4 mvp = camera->projection * camera->view * model;
			
			auto& group = spriteGroups.back();
			if (group.texture == spriteComp.texture) {
				++group.count;
			} else {
				spriteGroups.push_back({spriteComp.texture, 1, static_cast<GLuint>(instanceData.size())});
			}

			instanceData.push_back(InstanceData{mvp});
		}

		#if defined(DEBUG_GRAPHICS)
			if (instanceData.size() >= MAX_SPRITES) {
				ENGINE_WARN("Increase SpriteSystem::MAX_SPRITES. Attempting to draw " << instanceData.size() << " while MAX_SPRITES = " << MAX_SPRITES);
			}
		#endif

		// Update data
		glNamedBufferSubData(ivbo, 0, instanceData.size() * sizeof(InstanceData), instanceData.data());

		// Draw
		for (std::size_t i = 1; i < spriteGroups.size(); ++i) {
			const auto& group = spriteGroups[i];

			// Set texture
			glBindTextureUnit(0, group.texture);
			glUniform1i(6, 0);

			// Draw our sprites
			glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, group.count, group.base);
		}
	}
}
