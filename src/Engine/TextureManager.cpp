// STD
#include <algorithm>

// Engine
#include <Engine/TextureManager.hpp>
#include <Engine/Engine.hpp>
#include <Engine/TextureOptions.hpp>

// SOIL
#include <soil/SOIL.h>

namespace Engine {
	Texture TextureManager::load(const std::string& path) {
		int width;
		int height;
		int channels;
		auto image = SOIL_load_image(path.c_str(), &width, &height, &channels, SOIL_LOAD_RGBA);

		Texture tex;

		if (image == nullptr) {
			auto res = SOIL_last_result();

			ENGINE_WARN("[SOIL] ", res);

			width = 2;
			height = 2;
			image = new GLubyte[2 * 2 * 3] {
				255, 000, 000,    000, 255, 000,
				000, 255, 000,    255, 000, 000,
			};

			tex.setStorage(TextureFormat::SRGB8, 1, {width, height});
			tex.setSubImage(1, {0, 0}, {width, height}, PixelFormat::RGB8, image);

			delete[] image;
		} else {
			// Flip Y
			const int rowLength = width * channels;
			for (int y = 0; y < height / 2; ++y) {
				std::swap_ranges(
					image + y * rowLength,
					image + (y + 1) * rowLength,
					image + (height - y - 1) * rowLength
				);
			}
					
			tex.setStorage(TextureFormat::SRGBA8, 1, {width, height});
			tex.setSubImage(0, {0, 0}, {width, height}, PixelFormat::RGBA8, image);
			SOIL_free_image_data(image);
		}

		tex.setFilter(TextureFilter::NEAREST);
		tex.setWrap(TextureWrap::REPEAT);

		return tex;
	}
}
