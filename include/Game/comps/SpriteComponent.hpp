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
			Engine::TextureRef texture;
			glm::vec2 position = {0.0f, 0.0f};
			glm::vec2 scale = {1.0f, 1.0f};

			constexpr static auto netRepl() { return Engine::Net::Replication::ONCE; };

			void netToInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Engine::Net::BufferWriter& buff) const {
				buff.write(texture.id());
				buff.write(position);
				buff.write(scale);
			}

			void netFromInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Connection& conn) {
				const auto* tex = conn.read<Engine::TextureRef::Id>();
				if (!tex) {
					ENGINE_WARN("Unable to read sprite texture from network.");
					return;
				}
				
				const auto* pos = conn.read<glm::vec2>();
				if (!pos) {
					ENGINE_WARN("Unable to read sprite position from network.");
					return;
				}

				const auto* sc = conn.read<glm::vec2>();
				if (!sc) {
					ENGINE_WARN("Unable to read sprite scale from network.");
					return;
				}

				texture = engine.textureManager.get(*tex);
				position = *pos;
				scale = *sc;
			}
	};
}
