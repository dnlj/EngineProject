// Game
#include <Game/comps/PhysicsBodyComponent.hpp>
#include <Game/World.hpp>


namespace Game {
	void PhysicsBodyComponent::setBody(b2Body* body) {
		this->body = body;
	}

	void PhysicsBodyComponent::netTo(Engine::Net::BufferWriter& buff) const {
		buff.write(getTransform());
		buff.write(getVelocity());
	}

	void PhysicsBodyComponent::netToInit(EngineInstance& engine, World& world, Engine::ECS::Entity ent, Engine::Net::BufferWriter& buff) const {
		buff.write(type);
		buff.write(body->GetType());

		if (const auto* fix = body->GetFixtureList()) {
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

		netTo(buff);
	}

	void PhysicsBodyComponent::netFrom(Connection& conn) {
		const auto trans = *conn.read<b2Transform>();
		const auto vel = *conn.read<b2Vec2>();
		setTransform(trans.p, trans.q.GetAngle());
		rollbackOverride = true;
	}

	void PhysicsBodyComponent::netFromInit(EngineInstance& engine, World& world, Engine::ECS::Entity ent, Connection& conn) {
		const auto* ptype = conn.read<PhysicsType>();
		if (!ptype) {
			ENGINE_WARN("Unable to read physics type from network.");
			return;
		}
		type = *ptype;

		const auto* btype = conn.read<b2BodyType>();
		if (!btype) {
			ENGINE_WARN("Unable to read b2 physics type from network.");
			return;
		}

		const auto* ftype = conn.read<b2Shape::Type>();
		if (!ftype) {
			ENGINE_WARN("Unable to read physics fixture type from network.");
			return;
		}

		const auto* mask = conn.read<uint16>();
		if (!mask) {
			ENGINE_WARN("Unable to read physics filter mask from network");
			return;
		}

		auto& physSys = world.getSystem<PhysicsSystem>();
		{
			b2BodyDef bodyDef;
			bodyDef.fixedRotation = true;
			bodyDef.linearDamping = 10.0f; // TODO: value?
			bodyDef.type = *btype;
			bodyDef.position = {0, 0}; // TODO: read correct position from network

			setBody(physSys.createBody(ent, bodyDef));
		}

		b2FixtureDef fixDef;
		fixDef.filter.groupIndex = -+type;
		fixDef.filter.maskBits = *mask;

		switch (*ftype) {
			case b2Shape::Type::e_circle: {
				const auto* radius = conn.read<float32>();
				if (!radius) {
					ENGINE_WARN("Unable to read physics fixture radius from network.");
					return;
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
					return;
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
						return;
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

		netFrom(conn);
	}
}
