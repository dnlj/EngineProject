#pragma once

// STD
#include <string_view>

// glLoadGen
#include <glloadgen/gl_core_4_5.h>

// Engine
#include <Engine/TextureOptions.hpp>

namespace Engine {
	class Texture {
		public:
			/**
			 * @brief Creates a texture from the image at a given path with the given options.
			 * @param[in] path The path to the image.
			 * @param[in] options The options for the texture.
			 */
			Texture(const std::string& path, TextureOptions options);

			/**
			 * @brief Deconstructs the texture including the OpenGL object.
			 */
			~Texture();

			/**
			 * @brief Gets the OpenGL texture id.
			 * @return The OpenGL texture id.
			 */
			GLuint getID() const;

		private:
			/** The OpenGL texture id. */
			GLuint texture;
	};
}