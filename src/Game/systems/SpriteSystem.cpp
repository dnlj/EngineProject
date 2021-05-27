// GLM
#include <glm/gtc/matrix_transform.hpp>

// Box2D
#include <Box2D/b2_math.h>

// Engine
#include <Engine/Utility/Utility.hpp>
#include <Engine/ECS/EntityFilter.hpp>
#include <Engine/Glue/glm.hpp>

// Game
#include <Game/systems/SpriteSystem.hpp>
#include <Game/comps/SpriteComponent.hpp>
#include <Game/World.hpp>


namespace Game {
	SpriteSystem::SpriteSystem(SystemArg arg)
		: System{arg} {
		static_assert(World::orderAfter<SpriteSystem, PhysicsSystem>());
		
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

	SpriteSystem::~SpriteSystem() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ivbo);
		glDeleteBuffers(1, &ebo);
	}

	void SpriteSystem::run(float dt) {
		auto& filter = world.getFilter<
			Game::SpriteComponent,
			Game::PhysicsInterpComponent
		>();
		if (filter.empty()) { return; }

		// TODO: Look into array textures (GL_TEXTURE_2D_ARRAY)

		for (const auto& ent : filter) {
			const glm::vec3 pos = {Engine::Glue::as<glm::vec2>(world.getComponent<Game::PhysicsInterpComponent>(ent).getPosition()), 0.0f};
			const auto& spriteComp = world.getComponent<Game::SpriteComponent>(ent);

			sprites.push_back({
				.texture = spriteComp.texture->tex.get(),
				.trans = glm::scale(
					glm::translate(glm::mat4{1.0f}, pos + glm::vec3{spriteComp.position, 0.0f}),
					glm::vec3{spriteComp.scale, 1.0f}
				),
			});
		}

		std::sort(sprites.begin(), sprites.end(), [](const Sprite& a, const Sprite& b) {
			return a.texture < b.texture;
		});

		// Populate data
		spriteGroups.emplace_back();

		for (const auto& sprite : sprites) {
			// Set camera uniform
			glm::mat4 mvp = engine.camera.getProjection() * engine.camera.getView() * sprite.trans;
			
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
				ENGINE_WARN("Increase SpriteSystem::MAX_SPRITES. Attempting to draw ", instanceData.size(), " while MAX_SPRITES = ", MAX_SPRITES);
			}
		#endif

		// Update data
		glNamedBufferSubData(ivbo, 0, instanceData.size() * sizeof(InstanceData), instanceData.data());

		// VAO / Program
		glBindVertexArray(vao);
		glUseProgram(*shader);

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
