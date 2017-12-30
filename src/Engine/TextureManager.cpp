// Engine
#include <Engine/TextureManager.hpp>
#include <Engine/Engine.hpp>
#include <Engine/TextureOptions.hpp>

// SOIL
#include <SOIL.h>

namespace Engine {
	TextureManager::TextureManager() {
		textures.max_load_factor(0.5f);
	}

	TextureManager::~TextureManager() {
		for (const auto& texture : textures) {
			glDeleteTextures(1, &texture.second);
		}
	}

	GLuint TextureManager::getTexture(const std::string& path) {
		auto& texture = textures[path];

		if (texture != 0) { return texture; }

		Engine::TextureOptions options{Engine::TextureWrap::REPEAT, Engine::TextureFilter::NEAREST, false};

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		int width;
		int height;
		auto image = SOIL_load_image(path.c_str(), &width, &height, nullptr, SOIL_LOAD_RGBA);

		if (image == nullptr) {
			auto res = SOIL_last_result();

			ENGINE_WARN("[SOIL] " << res);

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
}