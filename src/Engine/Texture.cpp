// Engine
#include <Engine/Texture.hpp>
#include <Engine/Engine.hpp>
#include <Engine/Debug/Debug.hpp>

// SOIL
#include <SOIL.h>

namespace Engine {
	Texture::Texture(const std::string& path, TextureOptions options) {
		load(path, options);
	}

	void Texture::load(const std::string& path, TextureOptions options) {
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
			image = new GLubyte[4 * 4] {
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

		#if defined(DEBUG)
				Debug::checkOpenGLErrors();
		#endif
	}

	Texture::~Texture() {
		glDeleteTextures(1, &texture);
	}

	GLuint Texture::getID() const {
		return texture;
	}
}