#pragma once

// STD
#include <unordered_map>

// Engine
#include <Engine/ResourceManager.hpp>
#include <Engine/Graphics/Texture.hpp>


namespace Engine {
	class TextureManager : public Engine::ResourceManager<TextureManager, Texture> {
		friend class Engine::ResourceManager<TextureManager, Texture>;
		private:
			Texture load(const std::string& path);
			void unload(Texture tex) {}
	};

	using TextureRef = TextureManager::Resource;
}
