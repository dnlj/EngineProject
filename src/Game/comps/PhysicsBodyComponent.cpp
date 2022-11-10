// Game
#include <Game/comps/PhysicsBodyComponent.hpp>
#include <Game/World.hpp>


namespace Game {
	void PhysicsBodyComponent::setBody(b2Body* body) {
		this->body = body;
	}

	void NetworkTraits<PhysicsBodyComponent>::write(const PhysicsBodyComponent& obj, Engine::Net::BufferWriter& buff) {
		buff.write(obj.getTransform());
		buff.write(obj.getVelocity());
	}

	void NetworkTraits<PhysicsBodyComponent>::writeInit(const PhysicsBodyComponent& obj, Engine::Net::BufferWriter& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent) {
		buff.write(obj.type);
		buff.write(obj.getBody().GetType());

		if (const auto* fix = obj.getBody().GetFixtureList()) {
			const auto ftype = fix->GetType();
			buff.write(ftype);
			buff.write(fix->GetFilterData().maskBits);

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

		NetworkTraits::write(obj, buff);
	}

	void NetworkTraits<PhysicsBodyComponent>::read(PhysicsBodyComponent& obj, Connection& conn) {
		const auto trans = *conn.read<b2Transform>();
		const auto vel = *conn.read<b2Vec2>();
		obj.setTransform(trans.p, trans.q.GetAngle());
		obj.rollbackOverride = true;
	}

	std::tuple<PhysicsBodyComponent> NetworkTraits<PhysicsBodyComponent>::readInit(Connection& conn, EngineInstance& engine, World& world, Engine::ECS::Entity ent) {
		PhysicsBodyComponent result;

		const auto* ptype = conn.read<PhysicsType>();
		if (!ptype) {
			ENGINE_WARN("Unable to read physics type from network.");
			return result;
		}
		result.type = *ptype;

		const auto* btype = conn.read<b2BodyType>();
		if (!btype) {
			ENGINE_WARN("Unable to read b2 physics type from network.");
			return result;
		}

		const auto* ftype = conn.read<b2Shape::Type>();
		if (!ftype) {
			ENGINE_WARN("Unable to read physics fixture type from network.");
			return result;
		}

		const auto* mask = conn.read<uint16>();
		if (!mask) {
			ENGINE_WARN("Unable to read physics filter mask from network");
			return result;
		}

		auto& physSys = world.getSystem<PhysicsSystem>();
		b2Body* body = nullptr;
		{
			b2BodyDef bodyDef;
			bodyDef.fixedRotation = true;
			bodyDef.linearDamping = 10.0f; // TODO: value?
			bodyDef.type = *btype;
			bodyDef.position = {0, 0}; // TODO: read correct position from network
			body = physSys.createBody(ent, bodyDef);
			result.setBody(body);
		}

		b2FixtureDef fixDef;
		fixDef.filter.groupIndex = -+result.type;
		fixDef.filter.maskBits = *mask;

		switch (*ftype) {
			case b2Shape::Type::e_circle: {
				const auto* radius = conn.read<float32>();
				if (!radius) {
					ENGINE_WARN("Unable to read physics fixture radius from network.");
					return result;
				}

				b2CircleShape shape;
				shape.m_radius = *radius;
				
				fixDef.shape = &shape;
				body->CreateFixture(&fixDef);

				break;
			}
			case b2Shape::Type::e_polygon: {
				const auto* count = conn.read<uint8>();
				if (!count) {
					ENGINE_WARN("Unable to read physics fixture count from network.");
					return result;
				}

				if (*count != 4) {
					ENGINE_WARN("Physics fixture received from network with non-rect shape. This is probably an error.");
				}

				ENGINE_DEBUG_ASSERT(*count <= b2_maxPolygonVertices);
				b2Vec2 verts[b2_maxPolygonVertices];
				for (int i = 0; i < *count; ++i) {
					const auto* v = conn.read<b2Vec2>();;
					if (!v) {
						ENGINE_WARN("Unable to read vertex for physics shape from network.");
						return result;
					}
					verts[i] = *v;
				}

				b2PolygonShape shape;
				shape.Set(verts, *count);

				fixDef.shape = &shape;
				body->CreateFixture(&fixDef);

				break;
			}
			default: {
				ENGINE_WARN("Attempting to network unsupported physics fixture type.");
			}
		}

		NetworkTraits::read(result, conn);
		return std::make_tuple(result);
	}
}
