// Game
#include <Game/World.hpp>
#include <Game/comps/PhysicsBodyComponent.hpp>


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

namespace Game {
	void PhysicsBodyComponent::moveZone(WorldAbsVec oldZoneOffset, ZoneId newZoneId, WorldAbsVec newZoneOffset) {
		PhysicsBody::moveZone(oldZoneOffset, newZoneId, newZoneOffset);

		// Avoid interpolating after zone changes or else things will appear
		// very off (likely blank/off screen) for a few frames because it will
		// be something like lerp(1000, 0) = 500. Whereas in global coordinates
		// the position hasn't changed.
		snap = true;
	}

	void NetworkTraits<PhysicsBodyComponent>::write(const PhysicsBodyComponent& obj, Engine::Net::BufferWriter& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent) {
		buff.write<b2Transform>(obj.getTransform());
		buff.write<b2Vec2>(obj.getVelocity());
		buff.write<ZoneId>(obj.getZoneId());
	}

	void NetworkTraits<PhysicsBodyComponent>::writeInit(const PhysicsBodyComponent& obj, Engine::Net::BufferWriter& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent) {
		buff.write(obj.getType());

		if (const auto* fix = obj.getFixtureList()) {
			// Type
			const auto ftype = fix->GetType();
			buff.write(ftype);

			// Category
			static_assert(+PhysicsCategory::_count <= std::numeric_limits<uint8>::max());
			buff.write(static_cast<uint8>(PhysicsSystem::getCategory(fix->GetFilterData().categoryBits)));

			// Shape
			switch (ftype) {
				case b2Shape::Type::e_circle: {
					const auto* shape = reinterpret_cast<const b2CircleShape*>(fix->GetShape());
					buff.write(shape->m_radius);
					break;
				}
				case b2Shape::Type::e_polygon: {
					const auto* shape = reinterpret_cast<const b2PolygonShape*>(fix->GetShape());

					if constexpr (ENGINE_DEBUG) {
						if (shape->m_count != 4) {
							ENGINE_WARN("Attempting to network non-rect polygon physics shape. This is probably a error.");
						}
					}

					static_assert(b2_maxPolygonVertices < std::numeric_limits<uint8>::max());
					ENGINE_DEBUG_ASSERT(shape->m_count > 0);
					ENGINE_DEBUG_ASSERT(shape->m_count < std::numeric_limits<uint8>::max());
					buff.write(static_cast<uint8>(shape->m_count));

					for (int i = 0; i < shape->m_count; ++i) {
						const auto v = shape->m_vertices[i];
						buff.write(v);
					}
					break;
				}
				default: {
					ENGINE_WARN("Attempting to network unsupported physics fixture type.");
				}
			}
		} else {
			ENGINE_WARN("Attempting to network physics object without any fixtures.");
		}

		NetworkTraits::write(obj, buff, engine, world, ent);
	}

	void NetworkTraits<PhysicsBodyComponent>::read(PhysicsBodyComponent& obj, Engine::Net::BufferReader& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent) {
		b2Transform trans;
		buff.read<b2Transform>(&trans);

		b2Vec2 vel;
		buff.read<b2Vec2>(&vel);

		ZoneId zoneId;
		buff.read<ZoneId>(&zoneId);

		if (zoneId != obj.getZoneId()) {
			obj.setZone(zoneId);
		}

		obj.setTransform(trans.p, trans.q.GetAngle());
		obj.rollbackOverride = true;
	}

	std::tuple<PhysicsBodyComponent> NetworkTraits<PhysicsBodyComponent>::readInit(Engine::Net::BufferReader& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent) {
		PhysicsBodyComponent result;

		b2BodyType btype;
		if (!buff.read<b2BodyType>(&btype)) {
			ENGINE_WARN("Unable to read b2 physics type from network.");
			return result;
		}

		b2Shape::Type ftype;
		if (!buff.read<b2Shape::Type>(&ftype)) {
			ENGINE_WARN("Unable to read physics fixture type from network.");
			return result;
		}

		uint8 category;
		if (!buff.read<uint8>(&category)) {
			ENGINE_WARN("Unable to read physics filter mask from network");
			return result;
		}

		auto& physSys = world.getSystem<PhysicsSystem>();
		b2Body* body = nullptr;
		{
			b2BodyDef bodyDef;
			bodyDef.fixedRotation = true;
			bodyDef.linearDamping = 10.0f; // TODO: value?
			bodyDef.type = btype;
			bodyDef.position = {0, 0}; // TODO: read correct position from network
			body = physSys.createBody(ent, bodyDef);
			result.setBody(body, 0); // TODO: network zone
		}

		b2FixtureDef fixDef;
		fixDef.filter.categoryBits = PhysicsSystem::getCategoryBits(static_cast<PhysicsCategory>(category));
		fixDef.filter.maskBits = PhysicsSystem::getMaskBits(static_cast<PhysicsCategory>(category));

		switch (ftype) {
			case b2Shape::Type::e_circle: {
				b2CircleShape shape;
				if (!buff.read<float32>(&shape.m_radius)) {
					ENGINE_WARN("Unable to read physics fixture radius from network.");
					return result;
				}
				
				fixDef.shape = &shape;
				body->CreateFixture(&fixDef);

				break;
			}
			case b2Shape::Type::e_polygon: {
				uint8 count;
				if (!buff.read<uint8>(&count)) {
					ENGINE_WARN("Unable to read physics fixture count from network.");
					return result;
				}

				if (count != 4) {
					ENGINE_WARN("Physics fixture received from network with non-rect shape. This is probably an error.");
				}

				ENGINE_DEBUG_ASSERT(count <= b2_maxPolygonVertices);
				b2Vec2 verts[b2_maxPolygonVertices];
				for (int i = 0; i < count; ++i) {
					if (!buff.read<b2Vec2>(&verts[i])) {
						ENGINE_WARN("Unable to read vertex for physics shape from network.");
						return result;
					}
				}

				b2PolygonShape shape;
				shape.Set(verts, count);

				fixDef.shape = &shape;
				body->CreateFixture(&fixDef);

				break;
			}
			default: {
				ENGINE_WARN("Attempting to network unsupported physics fixture type.");
			}
		}

		NetworkTraits::read(result, buff, engine, world, ent);
		return std::make_tuple(std::move(result));
	}
}
