#pragma once

// Box2D
#include <Box2D/Box2D.h>

// Game
#include <Game/Zone.hpp>


// TODO: should this be in an Engine::Phys or similar?
namespace Game {
	class ZoneInfo {
		public:
			ZoneId id = zoneInvalidId;

			// TODO: should probably be a temp DS on the manager.
			ZoneId group = zoneInvalidId;
	};

	//
	//
	//
	// TODO: should this inherit MoveOnly? Probably
	//
	//
	//
	class PhysicsBody {
		private:
			b2Body* body = nullptr;

			// TODO: need to handle networking
		public:
			ZoneInfo zone; // TODO: make private once we have everyhing moved over.

		public:
			ENGINE_INLINE bool valid() const noexcept { return body; }
			void clear() noexcept;

			//
			//
			// TODO: should probably remove this function now that everything is dealing with PhysicsBody instead of b2
			//
			//
			void setBody(b2Body* body, ZoneId zoneId);
			void setZone(ZoneId zoneId);
			ENGINE_INLINE ZoneId getZoneId() const noexcept { return zone.id; }

			ENGINE_INLINE b2Body* takeOwnership() noexcept {
				const auto temp = body;
				*this = {};
				return temp;
			}
			
			void moveZone(WorldAbsVec oldZoneOffset, ZoneId newZoneId, WorldAbsVec newZoneOffset);

			ENGINE_INLINE const b2Fixture* getFixtureList() const noexcept { return body->GetFixtureList(); }

			ENGINE_INLINE const auto& getTransform() const noexcept { return body->GetTransform(); }
			ENGINE_INLINE void setTransform(const b2Vec2& pos, const float32 ang) { body->SetTransform(pos, ang); }

			ENGINE_INLINE auto getPosition() const noexcept { return body->GetPosition(); };
			ENGINE_INLINE void setPosition(const b2Vec2 p) noexcept { setTransform(p, getAngle()); };

			ENGINE_INLINE float32 getAngle() const noexcept { return body->GetAngle(); };
			ENGINE_INLINE void setAngle(const float32 a) noexcept { setTransform(getPosition(), a); };

			ENGINE_INLINE auto getVelocity() const noexcept { return body->GetLinearVelocity(); };
			ENGINE_INLINE void setVelocity(const b2Vec2 v) noexcept { body->SetLinearVelocity(v); };

			ENGINE_INLINE auto getAngularVelocity() const noexcept { return body->GetAngularVelocity(); };
			ENGINE_INLINE void setAngularVelocity(const float32 av) noexcept { body->SetAngularVelocity(av); };

			ENGINE_INLINE void applyLinearImpulse(const b2Vec2 imp, const bool wake) noexcept { body->ApplyLinearImpulseToCenter(imp, wake); }

			ENGINE_INLINE void setActive(bool active) { body->SetActive(active); }
			ENGINE_INLINE bool getActive(bool active) const noexcept { body->IsActive(); }

			ENGINE_INLINE b2BodyType getType() const noexcept { return body->GetType(); }

			ENGINE_INLINE void createFixture(b2FixtureDef def) {
				ENGINE_DEBUG_ASSERT(body != nullptr, "Attempting to create fixture for null body.");
				ENGINE_DEBUG_ASSERT(zone.id != zoneInvalidId, "Attempting to create fixture without a valid zone.");
				def.filter.groupIndex = zone.id;
				body->CreateFixture(&def);
			}
	};

}
