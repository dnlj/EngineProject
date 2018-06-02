// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// GLM
#include <glm/gtc/matrix_transform.hpp>

// Game
#include <Game/RenderSystem.hpp>
#include <Game/RenderComponent.hpp>
#include <Game/PhysicsComponent.hpp>

namespace Game {
	RenderSystem::RenderSystem(World& world) : SystemBase{world} {
		cbits = world.getBitsetForComponents<Game::RenderComponent, Game::PhysicsComponent>();

		priorityAfter = world.getBitsetForSystems<Game::PhysicsSystem>();
	}

	void RenderSystem::setup(Engine::Camera& camera) {
		this->camera = &camera;
	}

	void RenderSystem::run(float dt) {
		{
			// TODO: Move into different system
			const auto focusPos = world.getComponent<PhysicsComponent>(focus).body->GetPosition();
			camera->view = glm::translate(glm::mat4{1.0f}, glm::vec3{-focusPos.x, -focusPos.y, 0.0f});
		}

		for (auto& eid : entities) {
			const auto& rendComp = world.getComponent<Game::RenderComponent>(eid);
			const auto& physComp = world.getComponent<Game::PhysicsComponent>(eid);
		
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
				glm::mat4 mvp = camera->projection * camera->view * model;
				glUniformMatrix4fv(2, 1, GL_FALSE, &mvp[0][0]);
			}
		
			// Draw
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}

		#if defined (DEBUG_PHYSICS)
			world.getSystem<PhysicsSystem>().getDebugDraw().draw(camera->projection, camera->view);
		#endif
	}
}
