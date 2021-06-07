// Game
#include <Game/World.hpp>
#include <Game/systems/SubWorldSystem.hpp>


namespace {
	ENGINE_INLINE b2Body* copyBodyToWorld(b2Body* body, b2World* world) {
		b2BodyDef bodyDef;
		b2FixtureDef fixtureDef;
		bodyDef.type = body->GetType();
		bodyDef.position = body->GetPosition(); // TODO: this wont work. need to adjust for new world origin
		bodyDef.angle = body->GetAngle();
		bodyDef.linearVelocity = body->GetLinearVelocity();
		bodyDef.angularVelocity = body->GetAngularVelocity();
		bodyDef.linearDamping = body->GetLinearDamping();
		bodyDef.angularDamping = body->GetAngularDamping();
		bodyDef.allowSleep = body->IsSleepingAllowed();
		bodyDef.awake = body->IsAwake();
		bodyDef.fixedRotation = body->IsFixedRotation();
		bodyDef.bullet = body->IsBullet();
		bodyDef.active = body->IsActive();
		bodyDef.userData = body->GetUserData();
		bodyDef.gravityScale = body->GetGravityScale();

		auto* newBody = world->CreateBody(&bodyDef);
		auto* fixture = body->GetFixtureList();
		while (fixture) {
			fixtureDef.shape = fixture->GetShape();
			fixtureDef.userData = fixture->GetUserData();
			fixtureDef.friction = fixture->GetFriction();
			fixtureDef.restitution = fixture->GetRestitution();
			fixtureDef.density = fixture->GetDensity();
			fixtureDef.isSensor = fixture->IsSensor();
			fixtureDef.filter = fixture->GetFilterData();

			newBody->CreateFixture(&fixtureDef);
			fixture = fixture->GetNext();
		}

		return newBody;
	}
}

namespace Game {
	SubWorldSystem::SubWorldSystem(SystemArg arg)
		: System{arg} { // TODO: add own comp (flag comp?) to id players
	};

	void SubWorldSystem::tick() {
		// TODO: Lots can be improved here.

		// TODO: shoudl be able to do this whenever we merge/split so we dont need an extra iteration
		playerData.clear();
		for (auto ent : world.getFilter<PhysicsBodyComponent>()) {
			auto& physComp = world.getComponent<PhysicsBodyComponent>(ent);
			playerData.push_back({
				.ent = ent,
				.physComp = &physComp,
				.pos = physComp.getPosition(),
				.group = nullptr,
				.shouldSplit = false,
			});
		}
		const auto size = playerData.size();

		// Group and split players by proximity to eachother and world origins
		groups.clear();
		groups.reserve(size);
		for (int i = 0; i < size; ++i) {
			auto& ply1 = playerData[i];

			if (maxRangeSquared < ply1.pos.LengthSquared()) {
				ply1.shouldSplit = true;
			}

			if (ply1.group == nullptr) {
				groups.emplace_back();
				ply1.group = &groups.back();
			}

			for (int j = i + 1; j < size; ++j) {
				auto& ply2 = playerData[j];
				const auto diff = abs(ply1.pos.LengthSquared() - ply2.pos.LengthSquared());
				if (diff < minRangeSquared) {
					ply2.group = ply1.group;
					ply1.shouldSplit = false;
				}
			}
		}

		// Determine the most populated world per group
		for (int i = 0; i < size; ++i) {
			auto& ply = playerData[i];
			if (!ply.shouldSplit) {
				++ply.group->worlds[ply.physComp->getWorld()];
			}
		}

		// Set the world for each group
		for (auto& group : groups) { // TODO: this could be moved into the group find loop
			group.world = std::max_element(group.worlds.begin(), group.worlds.end(), [](const auto& a, const auto& b){
				return a.second < b.second;
			})->first;
		}
		// TODO: unused worlds arent freed

		// Merge worlds
		for (int i = 0; i < size; ++i) {
			auto& ply = playerData[i];
			if (!ply.shouldSplit && ply.physComp->getWorld() != ply.group->world) {
				mergePlayer(ply, ply.group->world);
			}
		}

		// TODO: could be merged into above
		// Split players
		for (int i = 0; i < size; ++i) {
			auto& ply = playerData[i];
			if (!ply.shouldSplit) { continue; }
			splitFromWorld(ply);
		}

		// TODO: where to handle shifting worlds?
	}

	void SubWorldSystem::mergePlayer(PlayerData& ply, b2World* world) {
		ENGINE_LOG("Merging - ", ply.ent, " ", world);
		ENGINE_DEBUG_ASSERT(ply.physComp->getWorld() != ply.group->world);
		auto* oldW = ply.physComp->getWorld();

		bodies.clear();
		struct QueryCallback : public b2QueryCallback {
			decltype(bodies)& bs;
			QueryCallback(decltype(bs) bs) : bs{bs} {};
			bool ReportFixture(b2Fixture* fixture) override {
				bs.insert(fixture->GetBody());
				return true;
			};
		} callback{bodies};

		oldW->QueryAABB(&callback, b2AABB{
			ply.pos - b2Vec2{minRange, minRange},
			ply.pos + b2Vec2{minRange, minRange},
		});

		b2BodyDef bodyDef;
		b2FixtureDef fixtureDef;
		for (auto* body : bodies) {
			Engine::ECS::Entity ent{static_cast<uint16>(reinterpret_cast<uintptr_t>(body->GetUserData())), 0};
			auto* newBody = copyBodyToWorld(body, world);
			this->world.getComponent<PhysicsBodyComponent>(ent).setBody(newBody);
		}

		// TODO: if oldW has no players add it to usable list for splitting
	}

	void SubWorldSystem::splitFromWorld(PlayerData& ply) {
		ENGINE_DEBUG_ASSERT(maxRangeSquared < ply.pos.LengthSquared(), "Needless player splitting");
		auto* world = getFreeWorld();
		ENGINE_LOG("Splitting - ", ply.ent, " ", world);
		if (world != ply.physComp->getWorld()) {
			mergePlayer(ply, world);
		}
		world->ShiftOrigin(ply.pos);
	}

	b2World* SubWorldSystem::getFreeWorld() {
		b2World* world;
		if (freeWorlds.empty()) {
			// TODO: need to copy world properities
			world = worlds.emplace_back(std::make_unique<b2World>(b2Vec2_zero)).get();
		} else {
			world = freeWorlds.back();
			freeWorlds.pop_back();
		}
		return world;
	}
}
