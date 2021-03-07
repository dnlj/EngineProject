#pragma once

// OpenGL
#include <glloadgen/gl_core_4_5.hpp>

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/TextureOptions.hpp>
#include <Engine/Graphics/Image.hpp>
#include <Engine/Graphics/TextureFormat.hpp>


namespace Engine {
	class Texture { // TODO: rename
		private:
			GLuint tex = 0;

		public:
			Texture() {}

			Texture(const Texture&) = delete;

			Texture(Texture&& other) {
				tex = other.tex;
				other.tex = 0;
			}

			~Texture() {
				glDeleteTextures(1, &tex);
			}

			void setStorage(TextureFormat format, glm::ivec2 size, int mips = 1) {
				if (tex == 0) { glCreateTextures(GL_TEXTURE_2D, 1, &tex); }
				glTextureStorage2D(tex, mips, static_cast<GLenum>(format), size.x, size.y);
			}
			
			void setImage(const Image& img) {
				setSubImage(0, img);
			}

			void setSubImage(int mip, const Image& img) {
				setSubImage(mip, {0, 0}, img);
			}

			void setSubImage(int mip, glm::ivec2 offset, const Image& img) {
				setSubImage(mip, offset, img.size(), img.format(), img.data());
			}

			void setSubImage(int mip, glm::ivec2 offset, glm::ivec2 size, PixelFormat format, const void* data) {
				ENGINE_DEBUG_ASSERT(tex != 0, "Attempting to set data of uninitialized texture.");
				// TODO: GL_UNSIGNED_BYTE will need to be a switch or something when we add more pixel formats
				const auto& pixInfo = getPixelFormatInfo(format);
				glTextureSubImage2D(tex, mip,
					offset.x, offset.y, size.x, size.y,
					pixInfo.glFormat, GL_UNSIGNED_BYTE, data
				);
			}

			void setMinFilter(TextureFilter filter) {
				decltype(auto) translate = [](auto f) ENGINE_INLINE -> GLenum {
					switch (f) {
						case TextureFilter::NEAREST: {
							return GL_NEAREST;
						}
						case TextureFilter::BILINEAR: {
							return GL_LINEAR;
						}
						case TextureFilter::TRILINEAR: {
							return GL_LINEAR_MIPMAP_LINEAR;
						}
					}
					ENGINE_WARN("Invalid texture minification filter.");
					return GL_NEAREST;
				};
				glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, translate(filter));
			}

			void setMagFilter(TextureFilter filter) {
				decltype(auto) translate = [](auto f) ENGINE_INLINE -> GLenum {
					switch (f) {
						case TextureFilter::NEAREST: {
							return GL_NEAREST;
						}
						case TextureFilter::BILINEAR:
						case TextureFilter::TRILINEAR: {
							return GL_LINEAR;
						}
					}
					ENGINE_WARN("Invalid texture magnification filter.");
					return GL_NEAREST;
				};
				glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, translate(filter));
			}

			void setFilter(TextureFilter filter) {
				setMinFilter(filter);
				setMagFilter(filter);
			}

			void setWrap(TextureWrap wrap) {
				glTextureParameteri(tex, GL_TEXTURE_WRAP_S, static_cast<GLenum>(wrap));
				glTextureParameteri(tex, GL_TEXTURE_WRAP_T, static_cast<GLenum>(wrap));
			}

			void generateMipmaps() {
				ENGINE_DEBUG_ASSERT(tex != 0, "Attempting to generate mipmaps for uninitialized texture.");
				glGenerateTextureMipmap(tex);
			}

			ENGINE_INLINE auto get() const noexcept { return tex; }
	};
}
