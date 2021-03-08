#pragma once

// STD
#include <unordered_map>

// Engine
#include <Engine/ResourceManager.hpp>
#include <Engine/Graphics/Texture.hpp>


namespace Engine {
	class TextureManager : public Engine::ResourceManager<TextureManager, Texture2D> {
		friend class Engine::ResourceManager<TextureManager, Texture2D>;
		private:
			Texture2D load(const std::string& path);
			void unload(Texture2D tex) {}
	};

	using TextureRef = TextureManager::Resource;
}
