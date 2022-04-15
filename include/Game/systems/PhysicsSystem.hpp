#pragma once

// Box2D
#include <Box2D/Box2D.h>

// Engine
#include <Engine/Debug/DebugDrawBox2D.hpp>

// Game
#include <Game/System.hpp>
#include <Game/PhysicsListener.hpp>


namespace Game {
	class PhysicsSystem : public System {
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
			b2Body* createPhysicsCircle(Engine::ECS::Entity ent, b2Vec2 position = b2Vec2_zero, int filter = 0) {
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
				fixtureDef.filter.groupIndex = filter;

				body->CreateFixture(&fixtureDef);
				return body;
			}

			ENGINE_INLINE static Engine::ECS::Entity toEntity(void* userdata) {
				return reinterpret_cast<Engine::ECS::Entity&>(userdata);
			};

		private:
			class ContactListener : public b2ContactListener {
				public:
					ContactListener(PhysicsSystem& physSys);
					virtual void BeginContact(b2Contact* contact) override;
					virtual void EndContact(b2Contact* contact) override;
					// virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
					// virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

					void addListener(PhysicsListener* listener);

				private:
					PhysicsSystem& physSys;
					std::vector<PhysicsListener*> listeners;
			};


			/** The box2d world */
			b2World physWorld;

			/** The box2d contact listener */
			ContactListener contactListener;

			#if defined(DEBUG_PHYSICS)
				Engine::Debug::DebugDrawBox2D debugDraw;
			#endif
	};
}
