#pragma once

// Box2D
#include <Box2D/Box2D.h>

// Engine
#include <Engine/Debug/DebugDrawBox2D.hpp>

// Game
#include <Game/System.hpp>
#include <Game/PhysicsUserData.hpp>
#include <Game/PhysicsListener.hpp>


namespace Game {
	class PhysicsSystem : public System {
		public:
			PhysicsSystem(SystemArg arg);

			void run(float dt);

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

			// TODO: Temp, move functions into PhysicsSystem
			b2World& getWorld() { return physWorld; };

			#if defined(DEBUG_PHYSICS)
				Engine::Debug::DebugDrawBox2D& getDebugDraw();
			#endif

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

			/**
			 * Converts from box2d user data to PhysicsUserData.
			 * @param[in] ptr The box2d user data.
			 * @return The PhysicsUserData.
			 */
			const PhysicsUserData& getUserData(void* ptr) const;

			/** The box2d world */
			b2World physWorld;

			/** The box2d contact listener */
			ContactListener contactListener;

			/** The user data to use for box2d */
			std::vector<PhysicsUserData> userData;

			#if defined(DEBUG_PHYSICS)
				Engine::Debug::DebugDrawBox2D debugDraw;
			#endif
	};
}
