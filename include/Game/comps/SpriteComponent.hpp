#pragma once

// STD
#include <string>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Engine
#include <Engine/Gfx/resources.hpp>
#include <Engine/Gfx/TextureManager.hpp>

// Game
#include <Game/RenderLayer.hpp>
#include <Game/NetworkTraits.hpp>


namespace Game {
	class SpriteComponent {
		public:
			std::string path; // TODO: hopefully temp. Ideally we would have a static asset id / prefab / w.e. that we already know about.
			Engine::Gfx::Texture2DRef texture;
			glm::vec2 position = {0.0f, 0.0f};
			glm::vec2 scale = {1.0f, 1.0f};
			RenderLayer layer = RenderLayer::Foreground;
	};

	template<>
	class NetworkTraits<SpriteComponent> {
		public:
			static Engine::Net::Replication getReplType(const SpriteComponent& obj) {
				return Engine::Net::Replication::ONCE;
			}

			static void writeInit(const SpriteComponent& obj, Engine::Net::BufferWriter& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent) {
				buff.write(engine.getTextureId(obj.path));
				buff.write(obj.position);
				buff.write(obj.scale);

				static_assert(RenderLayer::_count < RenderLayer{255}, "This code compresses layer to uint8");
				buff.write(static_cast<uint8>(obj.layer));
			}

			static void write(const SpriteComponent& obj, Engine::Net::BufferWriter& buff) {}

			static std::tuple<SpriteComponent> readInit(Engine::Net::BufferReader& buff, EngineInstance& engine, World& world, Engine::ECS::Entity ent) {
				SpriteComponent result;

				uint32 tex;
				if (!buff.read<uint32>(&tex)) {
					ENGINE_WARN("Unable to read sprite texture from network.");
					return {};
				}
				
				glm::vec2 pos;
				if (!buff.read<glm::vec2>(&pos)) {
					ENGINE_WARN("Unable to read sprite position from network.");
					return {};
				}

				glm::vec2 sc;
				if (!buff.read<glm::vec2>(&sc)) {
					ENGINE_WARN("Unable to read sprite scale from network.");
					return {};
				}

				uint8 lay;
				if (!buff.read<uint8>(&lay)) {
					ENGINE_WARN("Unable to read sprite layer from network.");
					return {};
				}

				result.texture = engine.getTextureLoader().get2D(engine.getTexturePath(tex));
				result.layer = static_cast<RenderLayer>(lay);
				result.position = pos;
				result.scale = sc;
				return result;
			}

			static void read(SpriteComponent& obj, Engine::Net::BufferReader& buff) {}
	};
}
