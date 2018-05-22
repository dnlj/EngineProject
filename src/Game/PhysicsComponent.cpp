// Game
#include <Game/PhysicsComponent.hpp>

namespace Game {
	void PhysicsComponent::setup(b2World& world) {
		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;

		body = world.CreateBody(&bodyDef);

		b2CircleShape shape;
		shape.m_radius = 1.0f/8.0f;

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &shape;
		fixtureDef.density = 1.0f;

		body->CreateFixture(&fixtureDef);
		body->SetLinearDamping(10.0f);
		body->SetFixedRotation(true);
	}
}
