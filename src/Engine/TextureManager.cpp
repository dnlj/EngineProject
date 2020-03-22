// STD
#include <algorithm>

// Engine
#include <Engine/TextureManager.hpp>
#include <Engine/Engine.hpp>
#include <Engine/TextureOptions.hpp>

// SOIL
#include <soil/SOIL.h>

namespace Engine {
	GLuint TextureManager::load(const std::string& path) {
		GLuint texture = 0;

		Engine::TextureOptions options{Engine::TextureWrap::REPEAT, Engine::TextureFilter::NEAREST, false};

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		int width;
		int height;
		int channels;
		auto image = SOIL_load_image(path.c_str(), &width, &height, &channels, SOIL_LOAD_RGBA);

		if (image == nullptr) {
			auto res = SOIL_last_result();

			ENGINE_WARN("[SOIL] ", res);

			width = 2;
			height = 2;
			image = new GLubyte[4 * 4]{
				255, 000, 000, 255,    000, 255, 000, 255,
				000, 255, 000, 255,    255, 000, 000, 255,
			};

			options.setFilter(TextureFilter::NEAREST);
			options.setWrap(TextureWrap::REPEAT);
			options.setMipmap(false);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
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

			glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
			SOIL_free_image_data(image);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, options.getGLWrap());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, options.getGLWrap());

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options.getGLFilterMin());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options.getGLFilterMag());

		if (options.getMipmap()) {
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		glBindTexture(GL_TEXTURE_2D, 0);

		return texture;
	}

	void TextureManager::unload(GLuint texture) {
		glDeleteTextures(1, &texture);
	}
}
