#pragma once

// Engine
#include <Engine/ResourceManager2.hpp>
#include <Engine/Gfx/Texture.hpp>


namespace Engine::Gfx {
	class TextureInfo {
		public:
			// TODO: no reason this needs to be a 2d texture, probably add a TextureGeneric that is the base for any texture.
			Texture2D tex;
			glm::ivec2 size;

			ENGINE_INLINE operator TextureHandle2D() const { return tex; }
	};

	class TextureManager : public ResourceManager2<TextureInfo> {
		using ResourceManager2::ResourceManager2;
	};

	using TextureRef = TextureManager::ResourceRef;

	class TextureLoader final : public ResourceLoader<std::string, TextureInfo> {
		using ResourceLoader::ResourceLoader;
		virtual Resource load(const Key& key) override {
			Image img = key;
			img.flipY();
			return {{img, TextureFilter::NEAREST, TextureWrap::REPEAT}, img.size()};
		}
	};
}
