// Engine
#include <Engine/Gfx/TextureManager.hpp>


namespace Engine::Gfx {
	auto TextureLoader::load(const Key& key) -> Resource {
		Image img = key;
		img.flipY();
		return {
			.tex = Texture2D{img, TextureFilter::Nearest, TextureWrap::Repeat},
			.size = {img.size(), 1},
			.type = TextureType::Target2D,
		};
	}

	Texture2DRef TextureLoader::get2D(const Key& key) {
		auto got = get(key);

		if (got->type != TextureType::Target2D) {
			ENGINE_WARN("Attempting to get texture as wrong type (", key, ").");
			ENGINE_DEBUG_ASSERT(false);
			return getErrorTexture2D();
		}

		return TextureGenericRef::unsafe_getInfo(got);
	}

	Texture2DRef TextureLoader::getErrorTexture2D() {
		if (err2D) { return err2D; }

		constexpr glm::ivec2 sz = {8,8};
		using Vec = glm::u8vec3;
		Image img = {PixelFormat::RGB8, sz};
		auto data = reinterpret_cast<Vec*>(img.data());

		for (int y = 0; y < sz.y; ++y) {
			for (int x = 0; x < sz.x; ++x) {
				data[y*sz.x+x] = x&1^y&1 ? Vec{255,0,255} : Vec{0,255,0};
			}
		}

		auto got = manager.create(TextureGenericInfo{
			Texture2D{img, TextureFilter::Nearest, TextureWrap::Repeat},
			{img.size(), 1},
			TextureType::Target2D,
		});

		err2D = TextureGenericRef::unsafe_getInfo(got);

		return err2D;
	}
}
