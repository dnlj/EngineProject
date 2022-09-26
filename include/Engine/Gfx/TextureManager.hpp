#pragma once

// Engine
#include <Engine/Gfx/resources.hpp>
#include <Engine/Gfx/Texture.hpp>
#include <Engine/ResourceManager.hpp>


namespace Engine {
	template<>
	class ResourceRef<Gfx::TextureGenericInfo> : public ResourceRefImpl<Gfx::TextureGenericInfo> {
		using ResourceRefImpl<Gfx::TextureGenericInfo>::ResourceRefImpl;
		public:
			// Allow conversion from any other texture ref.
			template<class Tex>
			ResourceRef(ResourceRef<Gfx::TextureInfo<Tex>>& other)
				: ResourceRef(ResourceRef<Gfx::TextureInfo<Tex>>::unsafe_getInfo(other)) {
			}
	};
}

namespace Engine::Gfx {
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
