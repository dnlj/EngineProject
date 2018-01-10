#pragma once

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

namespace Engine {
	enum class TextureWrap : GLint {
			REPEAT = GL_REPEAT,
			MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
			CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
	};

	enum class TextureFilter : GLint {
		NEAREST,
		BILINEAR,
		TRILINEAR,
		// TODO: Anisotropic
	};

	/**
	 * @brief A collection of options for textures.
	 */
	class TextureOptions {
		public:
			/**
			 * @brief Constructs a TextureOptions with the given options.
			 * @param[in] wrap The wrap mode.
			 * @param[in] filter The filter mode.
			 * @param[in] mipmap Should mipmaps be generated.
			 */
			TextureOptions::TextureOptions(TextureWrap wrap, TextureFilter filter, bool mipmap);

			/**
			 * @brief Sets the wrap mode.
			 * @param[in] wrap The wrap mode.
			 */
			void setWrap(TextureWrap wrap);

			/**
			 * @brief Sets the filter mode.
			 * @param[in] filter The filter mode.
			 */
			void setFilter(TextureFilter filter);

			/**
			 * @brief Sets if mipmaps should be generated.
			 * @param[in] mipmap Should mipmaps be generated.
			 */
			void setMipmap(bool mipmap);

			/**
			 * @brief Gets the wrap mode.
			 * @return The wrap mode.
			 */
			GLint getGLWrap() const;

			/**
			 * @brief Gets the filter mode for minification.
			 * @return The filter mode.
			 */
			GLint getGLFilterMin() const;

			/**
			 * @brief Gets the filter mode for magnification.
			 * @return The filter mode.
			 */
			GLint getGLFilterMag() const;

			/**
			 * @brief Gets if mipmaps should be generated.
			 * @return Should mipmaps be generated.
			 */
			bool getMipmap() const;

		private:
			TextureWrap wrap;
			TextureFilter filter;
			bool mipmap;
	};
}
