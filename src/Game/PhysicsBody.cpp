#include <Game/PhysicsBody.hpp>


namespace Game {
	void PhysicsBody::clear() noexcept {
		for (auto* fixture = body->GetFixtureList(); fixture;) {
			auto* next  = fixture->GetNext();
			body->DestroyFixture(fixture);
			fixture = next;
		}
	}

	void PhysicsBody::setBody(b2Body* body, ZoneId zoneId) {
		this->body = body;

		// TODO: remove, this is a workaround during transition. Bodies should
		// be setup correctly with their zoneId already before setting them.
		setZone(zoneId);

		// Assume that the body already has the correct zoneId
		if constexpr (ENGINE_DEBUG) {
			for (auto* fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
				auto filter = fixture->GetFilterData();
				ENGINE_DEBUG_ASSERT(filter.groupIndex == zone.id);
			}
		}
	}

	void PhysicsBody::setZone(ZoneId zoneId) {
		ENGINE_DEBUG_ASSERT(zoneId != zone.id);
		ENGINE_DEBUG_ASSERT(zoneId < static_cast<ZoneId>(std::numeric_limits<decltype(b2Filter::groupIndex)>::max()));
		zone.id = zoneId;

		for (auto* fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
			auto filter = fixture->GetFilterData();
			filter.groupIndex = zoneId;
			fixture->SetFilterData(filter);
		}
	}

	void PhysicsBody::moveZone(WorldAbsVec oldZoneOffset, ZoneId newZoneId, WorldAbsVec newZoneOffset) {
		ENGINE_DEBUG_ASSERT(newZoneId != getZoneId(), "Attempting to move PhysicsBody to the same zone.");
		const auto zoneOffsetDiff = newZoneOffset - oldZoneOffset;
		const b2Vec2 zoneOffsetDiffB2 = {static_cast<float32>(zoneOffsetDiff.x), static_cast<float32>(zoneOffsetDiff.y)};
		setPosition(getPosition() - zoneOffsetDiffB2);
		setZone(newZoneId);
	}
}
