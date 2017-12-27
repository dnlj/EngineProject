// Engine
#include <Engine/Texture.hpp>
#include <Engine/Engine.hpp>

// SOIL
#include <SOIL.h>

namespace Engine {
	Texture::Texture(const std::string& path) {
		int width;
		int height;
		int channels;
		auto image = SOIL_load_image(path.c_str(), &width, &height, &channels, SOIL_LOAD_RGBA);

		bool failedToLoad = false;
		if (image == nullptr) {
			failedToLoad = true;
			auto res = SOIL_last_result();

			ENGINE_WARN("[SOIL]" << res);

			width = 2;
			height = 2;
			channels = 4;
			image = new GLubyte[4 * 4]{
				255, 000, 000, 255,    000, 255, 000, 255,
				000, 255, 000, 255,    255, 000, 000, 255,
			};
			// TODO: Add texture options
		}

		glGenBuffers(1, &texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

		//glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);

		if (!failedToLoad) {
			SOIL_free_image_data(image);
		} else {
			delete[] image;
		}
	}

	Texture::~Texture() {
		glDeleteTextures(1, &texture);
	}

	GLuint Texture::getID() const {
		return texture;
	}
}