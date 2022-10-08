// GLM
#include <glm/gtc/matrix_transform.hpp>

// Box2D
#include <Box2D/b2_math.h>

// Engine
#include <Engine/Camera.hpp>
#include <Engine/ECS/EntityFilter.hpp>
#include <Engine/Gfx/ShaderManager.hpp>
#include <Engine/Glue/glm.hpp>
#include <Engine/Utility/Utility.hpp>
#include <Engine/Gfx/ResourceContext.hpp>
#include <Engine/Gfx/VertexAttributeLayout.hpp>

// Game
#include <Game/World.hpp>
#include <Game/systems/SpriteSystem.hpp>
#include <Game/comps/SpriteComponent.hpp>
#include <Game/comps/PhysicsInterpComponent.hpp>


namespace Game {
	SpriteSystem::SpriteSystem(SystemArg arg)
		: System{arg} {
		static_assert(World::orderAfter<SpriteSystem, PhysicsSystem>());
		auto& rctx = engine.getGraphicsResourceContext();

		shader = engine.getShaderLoader().get("shaders/sprite");

		if (!vertexLayout) {
			using namespace Engine::Gfx;
			const VertexAttributeLayoutDesc desc = {{
				{ .binding = 0, .divisor = 0 },
				{ .binding = 1, .divisor = 1 },
			},{
				{
					.input=0, .size=2, .type=NumberType::Float32, .target=VertexAttribTarget::Float, .normalize=false,
					.offset=offsetof(Vertex, position), .binding=0
				}, {
					.input=1, .size=2, .type=NumberType::Float32, .target=VertexAttribTarget::Float, .normalize=false,
					.offset=offsetof(Vertex, texCoord), .binding=0
				},{
					.input=2, .size=4, .type=NumberType::Float32, .target=VertexAttribTarget::Float, .normalize=false,
					.offset=offsetof(InstanceData, mvp) + 0*4*sizeof(float32), .binding=1
				},{
					.input=3, .size=4, .type=NumberType::Float32, .target=VertexAttribTarget::Float, .normalize=false,
					.offset=offsetof(InstanceData, mvp) + 1*4*sizeof(float32), .binding=1
				},{
					.input=4, .size=4, .type=NumberType::Float32, .target=VertexAttribTarget::Float, .normalize=false,
					.offset=offsetof(InstanceData, mvp) + 2*4*sizeof(float32), .binding=1
				},{
					.input=5, .size=4, .type=NumberType::Float32, .target=VertexAttribTarget::Float, .normalize=false,
					.offset=offsetof(InstanceData, mvp) + 3*4*sizeof(float32), .binding=1
				},
			}};

			vertexLayout = rctx.vertexLayoutManager.create(desc);
		}

		{ // Vertex buffer
			Vertex data[] = {
				Vertex{glm::vec2{-0.5f, +0.5f},   glm::vec2{+0.0f, +1.0f}},
				Vertex{glm::vec2{-0.5f, -0.5f},   glm::vec2{+0.0f, +0.0f}},
				Vertex{glm::vec2{+0.5f, -0.5f},   glm::vec2{+1.0f, +0.0f}},
				Vertex{glm::vec2{+0.5f, +0.5f},   glm::vec2{+1.0f, +1.0f}},
			};

			vertBuff = rctx.bufferManager.create();
			vertBuff->alloc(data);
			glVertexArrayVertexBuffer(vertexLayout->get(), dataBindingIndex, vertBuff->get(), 0, sizeof(Vertex));
		}

		{ // Instance vertex buffer
			instBuff = rctx.bufferManager.create();
			instanceData.reserve(128);
			resizeInstanceData();
		}

		{ // Element buffer
			GLubyte data[] = {
				0, 1, 2,
				2, 3, 0,
			};

			elemBuff = rctx.bufferManager.create();
			elemBuff->alloc(data);
			glVertexArrayElementBuffer(vertexLayout->get(), elemBuff->get());
		}
	}

	SpriteSystem::~SpriteSystem() {
	}

	void SpriteSystem::resizeInstanceData() {
		const auto size = instanceData.capacity();
		instBuff->alloc(size * sizeof(InstanceData), Engine::Gfx::StorageFlag::DynamicStorage);
		glVertexArrayVertexBuffer(vertexLayout->get(), instBindingIndex, instBuff->get(), 0, sizeof(InstanceData));
	}

	void SpriteSystem::update(float dt) {
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
		const auto vp = cam.getProjection() * cam.getView();
		for (const auto& sprite : sprites) {
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

			instanceData.emplace_back(vp * sprite.trans);
		}

		if (instanceData.capacity() > oldInstCap) {
			ENGINE_INFO("Resizing sprite instance buffer: ", oldInstCap, " > ", instanceData.capacity(), ")");
			resizeInstanceData();
		}

		// Update data
		instBuff->setData(instanceData);
		nextGroup = 0;
		nextLayer = spriteGroups.front().layer;
	}

	void SpriteSystem::render(const RenderLayer layer) {
		if (layer != nextLayer) { return; }

		// VAO / Program
		glBindVertexArray(vertexLayout->get());
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
