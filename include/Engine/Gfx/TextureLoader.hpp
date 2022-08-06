#pragma once

// Engine
#include <Engine/ResourceManager.hpp>
#include <Engine/Gfx/Texture.hpp>
#include <Engine/Gfx/resources.hpp>


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
			using ResourceLoader::ResourceLoader;
			using ResourceLoader::get;

			virtual Resource load(const Key& key) override {
				Image img = key;
				img.flipY();
				return {
					.tex = Texture2D{img, TextureFilter::Nearest, TextureWrap::Repeat},
					.size = {img.size(), 1},
					.type = TextureType::Target2D,
				};
			}

		public:
			Texture2DRef get2D(const Key& key) {
				auto got = get(key);

				if (got->type != TextureType::Target2D) {
					ENGINE_WARN("Attempting to get texture as wrong type (", key, ").");
					ENGINE_DEBUG_ASSERT(false);
					// TODO: return placeholder instead
				}

				return reinterpret_cast<Texture2DRef::ResourceInfo*>(
					TextureGenericRef::unsafe_getInfo(got)
				);
			}
	};
}
