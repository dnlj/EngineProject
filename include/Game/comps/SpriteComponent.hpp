#pragma once

// STD
#include <string>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Engine
#include <Engine/Gfx/resources.hpp>
#include <Engine/Gfx/TextureLoader.hpp>

// Game
#include <Game/comps/NetworkComponent.hpp>
#include <Game/RenderLayer.hpp>
#include <Game/EngineInstance.hpp> // TODO: split into cpp file and remove from header


namespace Game {
	class SpriteComponent : public NetworkComponent {
		public:
			std::string path; // TODO: hopefully temp. Ideally we would have a static asset id / prefab / w.e. that we already know about.
			Engine::Gfx::TextureRef texture;
			glm::vec2 position = {0.0f, 0.0f};
			glm::vec2 scale = {1.0f, 1.0f};
			RenderLayer layer = RenderLayer::Foreground;

			constexpr static auto netRepl() { return Engine::Net::Replication::ONCE; };

			void netToInit(EngineInstance& engine, World& world, Engine::ECS::Entity ent, Engine::Net::BufferWriter& buff) const {
				buff.write(engine.getTextureId(path));
				buff.write(position);
				buff.write(scale);

				static_assert(RenderLayer::_count < RenderLayer{255}, "This code compresses layer to uint8");
				buff.write(static_cast<uint8>(layer));
			}

			void netFromInit(EngineInstance& engine, World& world, Engine::ECS::Entity ent, Connection& conn) {
				const auto* tex = conn.read<uint32>();
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

				const auto* lay = conn.read<uint8>();
				if (!lay) {
					ENGINE_WARN("Unable to read sprite layer from network.");
					return;
				}

				texture = engine.getTextureLoader().get(engine.getTexturePath(*tex));
				layer = static_cast<RenderLayer>(*lay);
				position = *pos;
				scale = *sc;
			}
	};
}
