// Game
#include <Game/World.hpp>
#include <Game/SubWorldSystem.hpp>
#include <Game/CharacterMovementComponent.hpp>


namespace {
	ENGINE_INLINE void copyBodyToWorld(b2Body* body, b2World* world) {
		b2BodyDef bodyDef;
		b2FixtureDef fixtureDef;
		bodyDef.type = body->GetType();
		bodyDef.position = body->GetPosition();
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
	}
}

namespace Game {
	SubWorldSystem::SubWorldSystem(SystemArg arg)
		: System{arg}
		, playerFilter{world.getFilterFor<CharacterMovementComponent>()} { // TODO: add own comp (flag comp?) to id players
	};
	
	void SubWorldSystem::tick(float32 dt) {
		// TODO: Lots can be improved here.
		const auto size = playerData.size();

		// TODO: shoudl be able to do this whenever we merge so we dont need an extra iteration
		for (int i = 0; i < size; ++i) {
			auto& ply = playerData[i];
			ply.group = nullptr;
			ply.shouldSplit = false;
		}

		// Group and split players by proximity to eachother and world origins
		std::vector<Group> groups;
		groups.reserve(size);
		for (int i = 0; i < size; ++i) {
			auto& ply1 = playerData[i];

			if (maxRangeSquared < ply1.pos.LengthSquared()) {
				ply1.shouldSplit = true;
			}

			for (int j = i; j < size; ++j) {
				auto& ply2 = playerData[j];
				const auto diff = abs(ply1.pos.LengthSquared() - ply2.pos.LengthSquared());
				if (diff < minRangeSquared) {
					if (ply1.group == nullptr) {
						groups.emplace_back();
						ply1.group = &groups.back();
					}

					ply2.group = ply1.group;
					ply1.shouldSplit = false;
				}
			}
		}

		//std::sort(playerData.begin(), playerData.end(), [](const PlayerData& a, const PlayerData& b){
		//	return a.group < b.group;
		//});

		// Determine the most populated world per group
		for (int i = 0; i < size; ++i) {
			auto& ply = playerData[i];
			++ply.group->worlds[ply.physComp->getWorld()];
		}

		// Set the world for each group
		for (auto& group : groups) { // TODO: this could be moved into the group find loop
			group.world = std::max_element(group.worlds.begin(), group.worlds.end(), [](const auto& a, const auto& b){
				return a.second < b.second;
			})->first;
		}

		// Merge worlds
		for (int i = 0; i < size; ++i) {
			auto& ply = playerData[i];
			if (ply.world != ply.group->world) {
				mergePlayer(ply);
			}
		}

		// TODO: could be merged into above
		// Split players
		for (int i = 0; i < size; ++i) {
			auto& ply = playerData[i];
			if (!ply.shouldSplit) { continue; }
			ENGINE_DEBUG_ASSERT(ply.group == nullptr, "Attempting to split player that is part of group");
		}

		// TODO: where to handle shifting worlds?
	}

	void SubWorldSystem::mergePlayer(PlayerData& ply) {
		ENGINE_DEBUG_ASSERT(ply.physComp->getWorld() != ply.group->world);
		auto* oldW = ply.physComp->getWorld();
		auto* newW = ply.group->world;

		robin_hood::unordered_flat_set<b2Body*> bodies; // TODO: cache on object itself and just .clear()
		struct : public b2QueryCallback {
			decltype(bodies)& bodies = bodies;
			bool ReportFixture(b2Fixture* fixture) override {
				bodies.insert(fixture->GetBody());
				return true;
			};
		} callback;

		oldW->QueryAABB(&callback, b2AABB{
			ply.pos - b2Vec2{minRange, minRange},
			ply.pos + b2Vec2{minRange, minRange},
		});

		b2BodyDef bodyDef;
		b2FixtureDef fixtureDef;
		for (auto* body : bodies) {
			copyBodyToWorld(body, newW);
		}

		// TODO: if oldW has no players add it to usable list for splitting
	}

	void SubWorldSystem::splitFromWorld(PlayerData& ply) {
		ENGINE_DEBUG_ASSERT(maxRangeSquared < ply.pos.LengthSquared(), "Needless player splitting");
		// TODO: impl
		assert(false);
	}
}
