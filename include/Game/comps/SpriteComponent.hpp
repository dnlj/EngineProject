#pragma once

// STD
#include <string>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Game
#include <Game/comps/NetworkComponent.hpp>


namespace Game {
	class SpriteComponent : public NetworkComponent {
		public:
			Engine::Texture texture;

			constexpr static auto netRepl() { return Engine::Net::Replication::ONCE; };

			void netToInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Connection& conn) const {
				conn.write(texture.id());
			}

			void netFromInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Connection& conn) {
				const auto* tex = conn.read<Engine::Texture::Id>();
				ENGINE_WARN("**** SPRITE *********************");
				if (!tex) { return; }
				texture = engine.textureManager.get(*tex);
			}
	};
}
