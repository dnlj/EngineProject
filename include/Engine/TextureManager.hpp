#pragma once

// Engine
#include <Engine/ResourceManager.hpp>
#include <Engine/Graphics/Texture.hpp>


namespace Engine {
	struct TextureInfo {
		Texture2D tex;
		glm::ivec2 size;
	};

	class TextureManager : public Engine::ResourceManager<TextureManager, TextureInfo> {
		friend class Engine::ResourceManager<TextureManager, TextureInfo>;
		private:
			StorePtr load(const std::string& path);
			void unload(TextureInfo tex);
	};

	using TextureRef = TextureManager::Resource;
}
