// GLM
#include <glm/gtc/matrix_transform.hpp>

// Engine
#include <Engine/Utility/Utility.hpp>
#include <Engine/ECS/EntityFilter.hpp>

// Game
#include <Game/SpriteSystem.hpp>

namespace Game {
	SpriteSystem::SpriteSystem(World& world)
		: SystemBase{world}
		, filter{world.getFilterFor<
			Game::SpriteComponent,
			Game::PhysicsComponent>()}{

		priorityAfter = world.getBitsetForSystems<Game::MapRenderSystem>();
	}

	SpriteSystem::~SpriteSystem() {
		glDeleteProgram(shader);
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ivbo);
		glDeleteBuffers(1, &ebo);
	}

	void SpriteSystem::setup(const Engine::Camera& camera) {
		this->camera = &camera;

		// TODO: Split into multiple functions?

		{// Shader programs
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
			glCreateVertexArrays(1, &vao);
		}

		GLuint dataBindingIndex = 0;
		{ // Vertex buffer
			Vertex data[] = {
				Vertex{glm::vec2{-0.5f, +0.5f},   glm::vec2{+0.0f, +1.0f}},
				Vertex{glm::vec2{-0.5f, -0.5f},   glm::vec2{+0.0f, +0.0f}},
				Vertex{glm::vec2{+0.5f, -0.5f},   glm::vec2{+1.0f, +0.0f}},
				Vertex{glm::vec2{+0.5f, +0.5f},   glm::vec2{+1.0f, +1.0f}},
			};

			glCreateBuffers(1, &vbo);
			glNamedBufferData(vbo, sizeof(data), &data, GL_STATIC_DRAW);
			glVertexArrayVertexBuffer(vao, dataBindingIndex, vbo, 0, sizeof(Vertex));
		}

		GLuint instanceBindingIndex = 1;
		{ // Instance vertex buffer
			glCreateBuffers(1, &ivbo);
			glNamedBufferData(ivbo, MAX_SPRITES * sizeof(InstanceData), nullptr, GL_DYNAMIC_DRAW);
			glVertexArrayVertexBuffer(vao, instanceBindingIndex, ivbo, 0, sizeof(InstanceData));
			instanceData.reserve(MAX_SPRITES);
		}

		{ // Element buffer
			GLubyte data[] = {
				0, 1, 2,
				2, 3, 0,
			};

			glCreateBuffers(1, &ebo);
			glNamedBufferData(ebo, sizeof(data), &data, GL_STATIC_DRAW);
			glVertexArrayElementBuffer(vao, ebo);
		}

		{ // Vertex attributes
			// Vertex attributes
			glEnableVertexArrayAttrib(vao, 0);
			glEnableVertexArrayAttrib(vao, 1);

			glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
			glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texCoord));

			glVertexArrayAttribBinding(vao, 0, dataBindingIndex);
			glVertexArrayAttribBinding(vao, 1, dataBindingIndex);


			// Instance attributes
			glEnableVertexArrayAttrib(vao, 2);
			glEnableVertexArrayAttrib(vao, 3);
			glEnableVertexArrayAttrib(vao, 4);
			glEnableVertexArrayAttrib(vao, 5);

			glVertexArrayAttribFormat(vao, 2, 4, GL_FLOAT, GL_FALSE, offsetof(InstanceData, mvp) +  0 * sizeof(GLfloat));
			glVertexArrayAttribFormat(vao, 3, 4, GL_FLOAT, GL_FALSE, offsetof(InstanceData, mvp) +  4 * sizeof(GLfloat));
			glVertexArrayAttribFormat(vao, 4, 4, GL_FLOAT, GL_FALSE, offsetof(InstanceData, mvp) +  8 * sizeof(GLfloat));
			glVertexArrayAttribFormat(vao, 5, 4, GL_FLOAT, GL_FALSE, offsetof(InstanceData, mvp) + 12 * sizeof(GLfloat));

			glVertexArrayAttribBinding(vao, 2, instanceBindingIndex);
			glVertexArrayAttribBinding(vao, 3, instanceBindingIndex);
			glVertexArrayAttribBinding(vao, 4, instanceBindingIndex);
			glVertexArrayAttribBinding(vao, 5, instanceBindingIndex);

			glVertexArrayBindingDivisor(vao, instanceBindingIndex, 1);
		}
	}

	void SpriteSystem::run(float dt) {
		if (filter.empty()) { return; }

		// TODO: Look into array textures (GL_TEXTURE_2D_ARRAY)

		// Sort by texture
		entitiesByTexture.assign(filter.begin(), filter.end());

		std::sort(entitiesByTexture.begin(), entitiesByTexture.end(), [this](Engine::ECS::Entity a, Engine::ECS::Entity b) {
			return world.getComponent<Game::SpriteComponent>(a).texture < world.getComponent<Game::SpriteComponent>(b).texture;
		});

		// Populate data
		instanceData.clear();
		spriteGroups.clear();
		spriteGroups.emplace_back();

		for (auto& ent : entitiesByTexture) {
			const auto& spriteComp = world.getComponent<Game::SpriteComponent>(ent);
			const auto& physComp = world.getComponent<Game::PhysicsComponent>(ent);
			const auto& transform = physComp.body->GetTransform();

			// Set camera uniform
			auto model = glm::translate(glm::mat4{1.0f}, glm::vec3{transform.p.x, transform.p.y, 0.0f}) * glm::scale(glm::mat4{1.0f}, glm::vec3{1.0f/4});
			glm::mat4 mvp = camera->getProjection() * camera->getView() * model;
			
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

		// VAO / Program
		glBindVertexArray(vao);
		glUseProgram(shader);

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
