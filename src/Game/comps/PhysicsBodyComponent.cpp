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
		buff.write(obj.getBody().GetType());

		if (const auto* fix = obj.getBody().GetFixtureList()) {
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

		NetworkTraits::write(obj, buff);
	}

	void NetworkTraits<PhysicsBodyComponent>::read(PhysicsBodyComponent& obj, Engine::Net::BufferReader& buff) {
		b2Transform trans;
		buff.read<b2Transform>(&trans);

		b2Vec2 vel;
		buff.read<b2Vec2>(&vel);

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
			result.setBody(body);
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

		NetworkTraits::read(result, buff);
		return std::make_tuple(result);
	}
}
