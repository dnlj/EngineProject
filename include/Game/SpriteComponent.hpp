#pragma once

// STD
#include <string>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// Engine
#include <Engine/Net/Replication.hpp>


namespace Game {
	class SpriteComponent {
		public:
			Engine::Texture texture;

			constexpr static auto netRepl() { return Engine::Net::Replication::NONE; };
			
			void netTo(Engine::Net::PacketWriter& writer) const {}

			void netToInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Engine::Net::PacketWriter& writer) const {
				writer.write(texture.id());
			}

			void netFrom(Engine::Net::PacketReader& reader) {}

			void netFromInit(Engine::EngineInstance& engine, World& world, Engine::ECS::Entity ent, Engine::Net::PacketReader& reader) {
				const auto* tex = reader.read<Engine::Texture::Id>();
				if (!tex) { return; }
				texture = engine.textureManager.get(*tex);
			}
	};
}
