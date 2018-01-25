// Engine
#include <Engine/ECS/ECS.hpp>

// Game
#include <Game/PhysicsComponent.hpp>

namespace Game {
	void PhysicsComponent::setup(b2World& world) {
		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;

		body = world.CreateBody(&bodyDef);

		b2CircleShape shape;
		shape.m_radius = 0.25f;

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &shape;
		fixtureDef.density = 1.0f;

		body->CreateFixture(&fixtureDef);
	}
}

ENGINE_REGISTER_COMPONENT(Game::PhysicsComponent);
