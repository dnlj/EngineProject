#pragma once

// Box2D
#include <Box2D/Box2D.h>

// Engine
#include <Engine/Debug/DebugDrawBox2D.hpp>

// Game
#include <Game/System.hpp>
#include <Game/PhysicsListener.hpp>


namespace Game {
	enum class PhysicsCategory {
		World       = 0,
		Player      = 1,
		Decoration  = 2,
		_count,
	};
	ENGINE_BUILD_DECAY_ENUM(PhysicsCategory);

	class PhysicsSystem : public System, public b2ContactListener, public b2ContactFilter {
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

			/**
			 * Creates a box2d body and associates an entity with it.
			 * @param[in] ent The entity.
			 * @param[in] bodyDef The box2d body definition.
			 * @return A box2d body.
			 */
			b2Body* createBody(Engine::ECS::Entity ent, b2BodyDef& bodyDef);

			/**
			 * Destroys a box2d body.
			 * @param[in] body The body to destroy.
			 */
			void destroyBody(b2Body* body);

			/**
			 * Adds a physics listener.
			 * @param[in] listener The listener.
			 */
			void addListener(PhysicsListener* listener);

			/**
			 * Changes the origin of the physics world.
			 * @param[in] newOrigin The new origin relative to the current origin.
			 */
			void shiftOrigin(const b2Vec2& newOrigin);

			// TODO: rm - temp
			b2Body* createPhysicsCircle(Engine::ECS::Entity ent, b2Vec2 position, PhysicsCategory group) {
				b2BodyDef bodyDef;
				bodyDef.type = b2_dynamicBody;
				bodyDef.position = position;
				bodyDef.linearDamping = 10.0f;
				bodyDef.fixedRotation = true;

				b2Body* body = createBody(ent, bodyDef);

				b2CircleShape shape;
				shape.m_radius = 0.49f;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &shape;
				fixtureDef.density = 1.0f;
				fixtureDef.filter.categoryBits = getCategoryBits(group);
				fixtureDef.filter.maskBits = getMaskBits(group);

				body->CreateFixture(&fixtureDef);
				return body;
			}

			ENGINE_INLINE static Engine::ECS::Entity toEntity(const void* userdata) noexcept {
				return reinterpret_cast<Engine::ECS::Entity&>(userdata);
			};


			// TODO: We shouldn't expose getFilterCategory/getFilterMask. We
			//       should handle fixture creation so we have a single access point
			//       to maintain instead of creating them ad-hoc.
			using FilterBitset = decltype(b2Filter::categoryBits);
			
			constexpr static PhysicsCategory getCategory(FilterBitset category) noexcept {
				return static_cast<PhysicsCategory>(std::countr_zero(category));
			}

			constexpr static FilterBitset getCategoryBits(PhysicsCategory group) noexcept {
				return 1 << +group;
			}

			constexpr static FilterBitset getMaskBits(PhysicsCategory group) {
				constexpr auto makeFilterMask = [](std::initializer_list<PhysicsCategory> groups){
					FilterBitset result{};
					for (auto group : groups) {
						result |= getCategoryBits(group);
					}
					return result;
				};

				constexpr auto lookup = []() consteval noexcept {
					std::array<FilterBitset, 16> results;
					constexpr FilterBitset maskAll = -1;

					// By default everything collides with everything
					std::ranges::fill(results, maskAll);

					// Special cases
					results[+PhysicsCategory::Player] = makeFilterMask({PhysicsCategory::World});
					results[+PhysicsCategory::Decoration] = {};

					return results;
				}();

				return lookup[+group];
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
