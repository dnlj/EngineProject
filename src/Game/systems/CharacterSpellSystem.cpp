// STD
#include <algorithm>

// Game
#include <Game/systems/CharacterSpellSystem.hpp>
#include <Game/systems/PhysicsSystem.hpp>
#include <Game/comps/SpriteComponent.hpp>
#include <Game/World.hpp>


namespace Game {
	CharacterSpellSystem::CharacterSpellSystem(SystemArg arg)
		: System{arg} {
		static_assert(World::orderAfter<CharacterSpellSystem, CharacterMovementSystem>());
		static_assert(World::orderAfter<CharacterSpellSystem, PhysicsSystem>());
	}

	void CharacterSpellSystem::setup() {
		auto& physSys = world.getSystem<Game::PhysicsSystem>();
		physSys.addListener(this);

		constexpr std::size_t count = 10;
		missiles.reserve(count);
		
		for (int i = 0; i < count; ++i) {
			auto ent = missiles.emplace_back(world.createEntity(true));
			auto& [spriteComp, physBodyComp, physProxyComp, physInterpComp] = world.addComponents<
				Game::SpriteComponent,
				Game::PhysicsBodyComponent,
				Game::PhysicsProxyComponent,
				Game::PhysicsInterpComponent
			>(ent);

			{
				b2BodyDef bodyDef;
				bodyDef.type = b2_dynamicBody;
				bodyDef.position = b2Vec2_zero;

				physBodyComp.setBody(physSys.createBody(ent, bodyDef));

				b2CircleShape shape;
				shape.m_radius = 1.0f/8;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &shape;
				fixtureDef.density = 0.0f;
				fixtureDef.isSensor = true;

				physBodyComp.getBody().CreateFixture(&fixtureDef);
				physBodyComp.getBody().SetLinearDamping(0.0f);
				physBodyComp.getBody().SetFixedRotation(true);
				physBodyComp.getBody().SetActive(false);
			}

			spriteComp.texture = engine.textureManager.get("assets/fire.png");
			world.setEnabled(ent, false);
		}
	}

	void CharacterSpellSystem::fireMissile(const b2Vec2& pos, const b2Vec2& dir) {
		auto missile = missiles[currentMissile];

		world.setEnabled(missile, true);
		auto& physBodyComp = world.getComponent<PhysicsBodyComponent>(missile);
		auto& physProxyComp = world.getComponent<PhysicsProxyComponent>(missile);
		physProxyComp.snap = true;
		physBodyComp.setTransform2(pos, 0);

		auto& body = physBodyComp.getBody();
		body.SetActive(true);
		body.SetLinearVelocity(4.0f * dir);

		currentMissile = (currentMissile + 1) % missiles.size();
	}

	void CharacterSpellSystem::detonateMissile(Engine::ECS::Entity ent) {
		std::cout << "Boom: " << ent << "\n";
		world.setEnabled(ent, false);
		world.getComponent<PhysicsBodyComponent>(ent).getBody().SetActive(false);
	}

	void CharacterSpellSystem::tick() {

		for (const auto ent : world.getFilter<PhysicsBodyComponent, ActionComponent>()) {
			auto& actComp = world.getComponent<ActionComponent>(ent);

			if (actComp.getButton(Button::Attack1).pressCount) {
				auto& physComp = world.getComponent<PhysicsBodyComponent>(ent);
				const auto pos = physComp.getPosition();
				const b2Vec2 target = {actComp.getAxis(Axis::TargetX), actComp.getAxis(Axis::TargetY)};
				auto dir = target - pos;
				dir.Normalize();

				fireMissile(pos + 0.3f * dir, dir);
			}
		}

		if (toDestroy.empty()) { return; }

		std::sort(toDestroy.begin(), toDestroy.end());

		while (!toDestroy.empty()) {
			auto ent = toDestroy.back();
			toDestroy.pop_back();
			detonateMissile(ent);

			while(!toDestroy.empty() && toDestroy.back() == ent) {
				toDestroy.pop_back();
			}
		}
	}

	void CharacterSpellSystem::beginContact(const Engine::ECS::Entity& entA, const Engine::ECS::Entity& entB) {
		const auto minEnt = missiles.front();
		const auto maxEnt = missiles.back();

		if (entA <= maxEnt && entA >= minEnt) {
			toDestroy.push_back(entA);
		}

		if (entB <= maxEnt && entB >= minEnt) {
			toDestroy.push_back(entB);
		}
	}
}
