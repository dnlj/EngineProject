#pragma once

// Box2D
#include <box2d/b2_body.h>

// Game
#include <Game/Common.hpp>


namespace Game {
	class PhysicsProxyComponent {
		public:
			constexpr static bool isSnapshotRelevant = true;
			bool snap = false; // TODO: snap is never set...
			bool rollbackOverride = false;

			// TODO: pos and ang instead of trans? prolly
			b2Transform trans = {};
			b2Vec2 vel = {};
			float32 angVel = {};

			ENGINE_INLINE void store(const b2Body& body) {
				trans = body.GetTransform();
				vel = body.GetLinearVelocity();
				angVel = body.GetAngularVelocity();
			}

			ENGINE_INLINE void apply(b2Body& body) const {
				// We need to perform these checks because setting these will wake the body and 90% they haven't changed
				if (trans.p != body.GetTransform().p) { // TODO: also check r
					body.SetTransform(trans.p, trans.q.GetAngle());
				}
				if (vel != body.GetLinearVelocity()) {
					body.SetLinearVelocity(vel);
				}
				if (angVel != body.GetAngularVelocity()) {
					body.SetAngularVelocity(angVel);
				}
			}

			Engine::Net::Replication netRepl() const {
				// TODO: dont network static bodies?
				// Was: return (body->GetType() == b2_staticBody) ? Engine::Net::Replication::NONE : Engine::Net::Replication::ALWAYS;
				return Engine::Net::Replication::ALWAYS;
			}

			void netTo(Connection& conn) const {
				conn.write(trans);
				conn.write(vel);
			}

			void netToInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Connection& conn) const {
				netTo(conn);
			}

			void netFrom(Connection& conn) {
				trans = *conn.read<b2Transform>();
				vel = *conn.read<b2Vec2>();
				rollbackOverride = true;
			}

			void netFromInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Connection& conn) {
				//auto& physSys = world.getSystem<PhysicsSystem>();
				//// TODO: actual shape
				//setBody(physSys.createPhysicsCircle(ent));
				////body->SetType(b2_staticBody);
				//netFrom(conn);
				netFrom(conn);
			}
	};
}
