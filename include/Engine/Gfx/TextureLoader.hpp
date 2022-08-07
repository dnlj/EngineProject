#pragma once

// Engine
#include <Engine/Gfx/resources.hpp>
#include <Engine/Gfx/Texture.hpp>
#include <Engine/ResourceManager.hpp>


namespace Engine::Gfx {
	template<class Tex>
	class TextureInfo {
		public:
			Tex tex;
			glm::ivec3 size;
			TextureType type;
	};

	using TextureGenericInfo = TextureInfo<TextureGeneric>;
	using Texture1DInfo = TextureInfo<Texture1D>;
	using Texture2DInfo = TextureInfo<Texture2D>;
	using Texture3DInfo = TextureInfo<Texture3D>;
	using Texture1DArrayInfo = TextureInfo<Texture1DArray>;
	using Texture2DArrayInfo = TextureInfo<Texture2DArray>;

	class TextureManager : public ResourceManager<TextureGenericInfo> {
		using ResourceManager::ResourceManager;
	};

	class TextureLoader final : public ResourceLoader<std::string, TextureGenericInfo> {
		private:
			Texture2DRef err2D;

		public:
			Texture2DRef get2D(const Key& key);
			Texture2DRef getErrorTexture2D();

		private:
			using ResourceLoader::ResourceLoader;
			using ResourceLoader::get;
			virtual Resource load(const Key& key) override;

	};
}
