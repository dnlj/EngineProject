// Game
#include <Game/comps/PhysicsBodyComponent.hpp>
#include <Game/World.hpp>


namespace Game {
	PhysicsBodyComponent::~PhysicsBodyComponent() noexcept {
		// TODO: need to handle when removed from ent: body->setActive(false);
		// TODO: this wont work with copy constructor (rollback buffer)
		// TODO: better way to handle this

		if (count && --*count == 0) {
			ENGINE_DEBUG_ASSERT(body, "Incorrect reference counting. PhysicsBodyComponent::body was nullptr.");
			body->GetWorld()->DestroyBody(body);
			delete count;
		}
	}
	
	PhysicsBodyComponent::PhysicsBodyComponent(const PhysicsBodyComponent& other) noexcept {
		count = other.count;
		body = other.body;
		if (count) { ++*count; }
	}

	PhysicsBodyComponent::PhysicsBodyComponent(PhysicsBodyComponent&& other) noexcept {
		*this = std::move(other);
	}

	void PhysicsBodyComponent::operator=(const PhysicsBodyComponent& other) noexcept {
		auto copy{other};
		*this = std::move(copy);
	}

	void PhysicsBodyComponent::operator=(PhysicsBodyComponent&& other) noexcept {
		using std::swap;
		swap(count, other.count);
		swap(body, other.body);
	}

	void PhysicsBodyComponent::setBody(b2Body* body) {
		this->body = body;
		ENGINE_DEBUG_ASSERT(count == nullptr);
		count = new int{1};
	}

	void PhysicsBodyComponent::netTo(Engine::Net::BufferWriter& buff) const {
		buff.write(getTransform());
		buff.write(getVelocity());
	}

	void PhysicsBodyComponent::netToInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Engine::Net::BufferWriter& buff) const {
		buff.write(type);
		buff.write(body->GetType());

		if (const auto* fix = body->GetFixtureList()) {
			const auto ftype = fix->GetType();
			buff.write(ftype);

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

					// TODO: uint8 max size is b2_maxPolygonVertices - static assert
					buff.write(shape->m_count);
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

	void PhysicsBodyComponent::netFromInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Connection& conn) {
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

		auto& physSys = world.getSystem<PhysicsSystem>();
		{
			b2BodyDef bodyDef;
			bodyDef.fixedRotation = true;
			bodyDef.linearDamping = 10.0f; // TODO: value?
			bodyDef.type = *btype;
			bodyDef.position = {0, 0}; // TODO: read correct position from network

			setBody(physSys.createBody(ent, bodyDef));
		}

		// TODO: make some functinos for toNet and fromNet in a anon namespace at top for networking this stuff. this is way to tedious.
		
		b2FixtureDef fixDef;
		fixDef.filter.groupIndex = -+type;

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
				const auto* count = conn.read<int32>();
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
