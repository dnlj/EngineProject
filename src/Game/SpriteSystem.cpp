// GLM
#include <glm/gtc/matrix_transform.hpp>

// Engine
#include <Engine/Utility/Utility.hpp>
#include <Engine/ECS/EntityFilter.hpp>

// Game
#include <Game/SpriteSystem.hpp>
#include <Game/PhysicsComponent.hpp>
#include <Game/SpriteComponent.hpp>
#include <Game/World.hpp>

namespace Game {
	SpriteSystem::SpriteSystem(SystemArg arg)
		: System{arg}
		, filter{world.getFilterFor<
			Game::SpriteComponent,
			Game::PhysicsComponent>()}{

		static_assert(World::orderAfter<SpriteSystem, PhysicsSystem>());
	}

	SpriteSystem::~SpriteSystem() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ivbo);
		glDeleteBuffers(1, &ebo);
	}

	void SpriteSystem::setup(Engine::EngineInstance& engine) {
		camera = &engine.camera;
		
		// TODO: Split into multiple functions?

		shader = engine.shaderManager.get("shaders/sprite");

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

		for (const auto& ent : filter) {
			const auto pos = world.getComponent<Game::PhysicsComponent>(ent).body->GetPosition();
			sprites.push_back({
				world.getComponent<Game::SpriteComponent>(ent).texture.get(),
				{pos.x, pos.y, 0.0f}
			});
		}

		std::sort(sprites.begin(), sprites.end(), [](const Sprite& a, const Sprite& b) {
			return a.texture < b.texture;
		});

		// Populate data
		spriteGroups.emplace_back();

		for (auto& sprite : sprites) {
			// Set camera uniform
			auto model = glm::translate(glm::mat4{1.0f}, sprite.position) * glm::scale(glm::mat4{1.0f}, glm::vec3{1.0f/4});
			glm::mat4 mvp = camera->getProjection() * camera->getView() * model;
			
			auto& group = spriteGroups.back();
			if (group.texture == sprite.texture) {
				++group.count;
			} else {
				spriteGroups.push_back({sprite.texture, 1, static_cast<GLuint>(instanceData.size())});
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
		glUseProgram(shader.get());

		// Draw
		for (std::size_t i = 1; i < spriteGroups.size(); ++i) {
			const auto& group = spriteGroups[i];

			// Set texture
			glBindTextureUnit(0, group.texture);
			glUniform1i(6, 0);

			// Draw our sprites
			glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, group.count, group.base);
		}

		// Cleanup
		sprites.clear();
		instanceData.clear();
		spriteGroups.clear();
	}

	void SpriteSystem::addSprite(Sprite sprite) {
		sprites.push_back(std::move(sprite));
	}
}
