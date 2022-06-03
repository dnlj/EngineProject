// GLM
#include <glm/gtc/matrix_transform.hpp>

// Box2D
#include <Box2D/b2_math.h>

// Engine
#include <Engine/Utility/Utility.hpp>
#include <Engine/ECS/EntityFilter.hpp>
#include <Engine/Glue/glm.hpp>
#include <Engine/Camera.hpp>

// Game
#include <Game/World.hpp>
#include <Game/systems/SpriteSystem.hpp>
#include <Game/comps/SpriteComponent.hpp>
#include <Game/comps/PhysicsInterpComponent.hpp>


namespace Game {
	SpriteSystem::SpriteSystem(SystemArg arg)
		: System{arg} {
		static_assert(World::orderAfter<SpriteSystem, PhysicsSystem>());

		shader = engine.getShaderManager().get("shaders/sprite");

		{ // Vertex array
			glCreateVertexArrays(1, &vao);
		}

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

		{ // Instance vertex buffer
			glCreateBuffers(1, &ivbo);
			instanceData.reserve(128);
			resizeInstanceData();
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

		{
			// Vertex attributes
			glEnableVertexArrayAttrib(vao, 0);
			glEnableVertexArrayAttrib(vao, 1);

			glVertexArrayAttribFormat(vao, 0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
			glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texCoord));

			glVertexArrayAttribBinding(vao, 0, dataBindingIndex);
			glVertexArrayAttribBinding(vao, 1, dataBindingIndex);


			// Instance attributes
			glVertexArrayBindingDivisor(vao, instBindingIndex, 1);

			glEnableVertexArrayAttrib(vao, 2);
			glEnableVertexArrayAttrib(vao, 3);
			glEnableVertexArrayAttrib(vao, 4);
			glEnableVertexArrayAttrib(vao, 5);

			glVertexArrayAttribFormat(vao, 2, 4, GL_FLOAT, GL_FALSE, offsetof(InstanceData, mvp) +  0 * sizeof(GLfloat));
			glVertexArrayAttribFormat(vao, 3, 4, GL_FLOAT, GL_FALSE, offsetof(InstanceData, mvp) +  4 * sizeof(GLfloat));
			glVertexArrayAttribFormat(vao, 4, 4, GL_FLOAT, GL_FALSE, offsetof(InstanceData, mvp) +  8 * sizeof(GLfloat));
			glVertexArrayAttribFormat(vao, 5, 4, GL_FLOAT, GL_FALSE, offsetof(InstanceData, mvp) + 12 * sizeof(GLfloat));

			glVertexArrayAttribBinding(vao, 2, instBindingIndex);
			glVertexArrayAttribBinding(vao, 3, instBindingIndex);
			glVertexArrayAttribBinding(vao, 4, instBindingIndex);
			glVertexArrayAttribBinding(vao, 5, instBindingIndex);
		}
	}

	SpriteSystem::~SpriteSystem() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ivbo);
		glDeleteBuffers(1, &ebo);
	}

	void SpriteSystem::resizeInstanceData() {
		const auto size = instanceData.capacity();
		glNamedBufferData(ivbo, size * sizeof(InstanceData), nullptr, GL_DYNAMIC_DRAW);
		glVertexArrayVertexBuffer(vao, instBindingIndex, ivbo, 0, sizeof(InstanceData));
	}

	void SpriteSystem::run(float dt) {
		auto& filter = world.getFilter<
			Game::SpriteComponent,
			Game::PhysicsInterpComponent
		>();
		if (filter.empty()) { return; }

		// Cleanup
		const auto oldInstCap = instanceData.capacity();
		sprites.clear();
		instanceData.clear();
		spriteGroups.clear();

		// TODO: Look into array textures (GL_TEXTURE_2D_ARRAY)

		for (const auto& ent : filter) {
			const glm::vec3 pos = {Engine::Glue::as<glm::vec2>(world.getComponent<Game::PhysicsInterpComponent>(ent).getPosition()), 0.0f};
			const auto& spriteComp = world.getComponent<Game::SpriteComponent>(ent);

			sprites.push_back({
				.layer = spriteComp.layer,
				.texture = spriteComp.texture->tex.get(),
				.trans = glm::scale(
					glm::translate(glm::mat4{1.0f}, pos + glm::vec3{spriteComp.position, 0.0f}),
					glm::vec3{spriteComp.scale, 1.0f}
				),
			});
		}

		std::sort(sprites.begin(), sprites.end(), [](const Sprite& a, const Sprite& b) ENGINE_INLINE {
			return (a.layer < b.layer) || (a.layer == b.layer && a.texture < b.texture);
		});

		// Populate data
		{
			const auto& sprite = sprites.front();
			spriteGroups.push_back({
				.layer = sprite.layer,
				.texture = sprite.texture,
			});
		}

		auto& cam = engine.getCamera();
		for (const auto& sprite : sprites) {
			// Set camera uniform
			glm::mat4 mvp = cam.getProjection() * cam.getView() * sprite.trans;
			
			auto& group = spriteGroups.back();
			if (group.layer == sprite.layer && group.texture == sprite.texture) {
				++group.count;
			} else {
				spriteGroups.push_back({
					.layer = sprite.layer,
					.texture = sprite.texture,
					.count = 1,
					.base = static_cast<GLuint>(instanceData.size()),
				});
			}

			instanceData.emplace_back(mvp);
		}

		if (instanceData.capacity() > oldInstCap) {
			ENGINE_INFO("Resizing sprite instance buffer: ", oldInstCap, " > ", instanceData.capacity(), ")");
			resizeInstanceData();
		}

		// Update data
		glNamedBufferSubData(ivbo, 0, instanceData.size() * sizeof(InstanceData), instanceData.data());
		nextGroup = 0;
		nextLayer = spriteGroups.front().layer;
	}

	void SpriteSystem::render(const RenderLayer layer) {
		if (layer != nextLayer) { return; }

		// VAO / Program
		glBindVertexArray(vao);
		glUseProgram(shader->get());

		const auto size = spriteGroups.size();
		const SpriteGroup* group = &spriteGroups[nextGroup];
		while (true) {
			// Set texture
			glBindTextureUnit(0, group->texture);
			glUniform1i(6, 0);

			// Draw our sprites
			glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, group->count, group->base);

			if (++nextGroup == size) {
				nextLayer = RenderLayer::_count;
				return;
			}

			group = &spriteGroups[nextGroup];
			if (group->layer != layer) {
				nextLayer = group->layer;
				return;
			}
		}
	}

	void SpriteSystem::addSprite(Sprite sprite) {
		sprites.push_back(std::move(sprite));
	}
}
