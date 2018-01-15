// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// GLM
#include <glm/gtc/matrix_transform.hpp>

// Game
#include <Game/RenderSystem.hpp>
#include <Game/RenderComponent.hpp>
#include <Game/PhysicsComponent.hpp>

namespace Game {
	decltype(projection) projection;
	decltype(view) view;

	RenderSystem::RenderSystem() {
		cbits = Engine::ECS::getBitsetForComponents<Game::RenderComponent, Game::PhysicsComponent>();

		// MVP
		constexpr float scale = 1.0f / 400.0f;
		auto halfWidth = (1280.0f / 2.0f) * scale;
		auto halfHeight = (720.0f / 2.0f) * scale;
		projection = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight);
		view = glm::mat4{1.0f};
	}

	void RenderSystem::run(float dt) {
		for (auto& ent : entities) {
			const auto& rendComp = ent.getComponent<Game::RenderComponent>();
			const auto& physComp = ent.getComponent<Game::PhysicsComponent>();

			glBindVertexArray(rendComp.vao);
			glUseProgram(rendComp.shader);

			// Texture
			// TODO: is this texture stuff stored in VAO?
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, rendComp.texture);
			glUniform1i(6, 0);

			// MVP
			{
				const auto& transform = physComp.body->GetTransform();
				auto model = glm::translate(glm::mat4{1}, glm::vec3{transform.p.x, transform.p.y, 0.0f});
				glm::mat4 mvp = projection * view * model;
				glUniformMatrix4fv(2, 1, GL_FALSE, &mvp[0][0]);
			}

			// Draw
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}

	ENGINE_REGISTER_SYSTEM(RenderSystem);
}
