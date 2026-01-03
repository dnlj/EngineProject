#pragma once

// Box2D
#include <Box2D/Box2D.h>

// Engine
#include <Engine/Debug/DebugDrawBox2D.hpp>

// Game
#include <Game/System.hpp>
#include <Game/PhysicsListener.hpp>
#include <Game/PhysicsBody.hpp>


namespace Game {
	enum class PhysicsCategory {
		World       = 0,
		Player      = 1,
		Decoration  = 2,
		Trigger     = 3,
		_count,
	};
	ENGINE_BUILD_DECAY_ENUM(PhysicsCategory);

	class PhysicsSystem : public System, public b2ContactListener, public b2ContactFilter {
		public:
			using FilterBitset = decltype(b2Filter::categoryBits);

		private:
			b2World physWorld;
			std::vector<PhysicsListener*> listeners;

		public:
			PhysicsSystem(SystemArg arg);

			void tick();
			void render(const RenderLayer layer);
			void preStoreSnapshot();

			void onComponentAdded(const Engine::ECS::Entity ent, class PhysicsBodyComponent& comp);
			void onComponentRemoved(const Engine::ECS::Entity ent, class PhysicsBodyComponent& comp);

			// TODO: really should just write accessors for whatever we need this for instead of leaking the entire world.
			const b2World& getPhysicsWorld() const noexcept { return physWorld; }

			/**
			 * Creates a box2d body and associates an entity with it.
			 * @param[in] ent The entity.
			 * @param[in] bodyDef The box2d body definition.
			 * @param[in] zoneId The initial zone for this body.
			 * @return A box2d body.
			 */
			PhysicsBody createBody(Engine::ECS::Entity ent, b2BodyDef& bodyDef, ZoneId zoneId);

			/**
			 * Destroys a box2d body.
			 * @param[in] body The body to destroy.
			 */
			void destroyBody(PhysicsBody& body);

			/**
			 * Adds a physics listener.
			 * @param[in] listener The listener.
			 */
			void addListener(PhysicsListener* listener);

			// TODO: rm - temp
			PhysicsBody createPhysicsRect(Engine::ECS::Entity ent, b2Vec2 position, glm::vec2 size, glm::vec2 offset, ZoneId zoneId, PhysicsCategory group) {
				b2BodyDef bodyDef;
				bodyDef.type = b2_dynamicBody;
				bodyDef.position = position;
				bodyDef.linearDamping = 10.0f;
				bodyDef.fixedRotation = true;

				auto body = createBody(ent, bodyDef, zoneId);

				b2PolygonShape shape;
				shape.SetAsBox(0.5f * size.x, 0.5f * size.y, {offset.x, offset.y}, 0);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &shape;
				fixtureDef.density = 1.0f;
				fixtureDef.filter.categoryBits = getCategoryBits(group);
				fixtureDef.filter.maskBits = getMaskBitsForCategory(group);

				body.createFixture(fixtureDef);
				return body;
			}

			ENGINE_INLINE static Engine::ECS::Entity toEntity(const b2Fixture* fixture) noexcept {
				return toEntity(fixture->GetBody()->GetUserData());
			}

			ENGINE_INLINE static Engine::ECS::Entity toEntity(const void* userdata) noexcept {
				return reinterpret_cast<Engine::ECS::Entity&>(userdata);
			};

			constexpr static PhysicsCategory getCategory(FilterBitset category) noexcept {
				return static_cast<PhysicsCategory>(std::countr_zero(category));
			}

			constexpr static FilterBitset getCategoryBits(PhysicsCategory category) noexcept {
				return 1 << +category;
			}

			constexpr static FilterBitset getMaskBitsForCategory(PhysicsCategory category) {
				constexpr auto makeFilterMask = [](std::initializer_list<PhysicsCategory> categories) consteval {
					FilterBitset result{};
					for (auto category : categories) {
						result |= getCategoryBits(category);
					}
					return result;
				};

				constexpr auto lookup = []() consteval noexcept {
					std::array<FilterBitset, +PhysicsCategory::_count> results;
					constexpr FilterBitset maskAll = -1;

					// By default everything collides with everything
					std::ranges::fill(results, maskAll);

					// Special cases
					results[+PhysicsCategory::Decoration] = {};
					results[+PhysicsCategory::Player] = makeFilterMask({PhysicsCategory::World, PhysicsCategory::Trigger});
					results[+PhysicsCategory::Trigger] = makeFilterMask({PhysicsCategory::Player});

					return results;
				}();

				return lookup[+category];
			}

			static b2FixtureDef getDefaultFixtureFor(PhysicsCategory category) noexcept {
				b2FixtureDef fixtureDef;

				// Filter group is setup when adding fixtures in PhysicsBody::createFixture
				// since the zone is a property of the body, not fixture.
				fixtureDef.filter.categoryBits = PhysicsSystem::getCategoryBits(category);
				fixtureDef.filter.maskBits = PhysicsSystem::getMaskBitsForCategory(category);

				if (category == PhysicsCategory::Trigger) {
					fixtureDef.isSensor = true;
				}

				return fixtureDef;
			}

		private:
			// b2ContactListener members
			virtual void BeginContact(b2Contact* contact) override;
			virtual void EndContact(b2Contact* contact) override;
			// virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
			// virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

			// b2ContactFilter members
			virtual bool ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB) override;

			// Debug members
			#if defined(DEBUG_PHYSICS)
				Engine::Debug::DebugDrawBox2D debugDraw;
			#endif
	};

	// Sanity check
	static_assert(PhysicsSystem::getCategory(PhysicsSystem::getCategoryBits(PhysicsCategory{0})) == PhysicsCategory{0});
	static_assert(PhysicsSystem::getCategory(PhysicsSystem::getCategoryBits(PhysicsCategory{8})) == PhysicsCategory{8});
	static_assert(PhysicsSystem::getCategory(PhysicsSystem::getCategoryBits(PhysicsCategory{16})) == PhysicsCategory{16});
	static_assert(PhysicsSystem::getCategory(PhysicsSystem::getCategoryBits(PhysicsCategory::World)) == PhysicsCategory::World);
	static_assert(PhysicsSystem::getCategory(PhysicsSystem::getCategoryBits(PhysicsCategory::Decoration)) == PhysicsCategory::Decoration);
}
