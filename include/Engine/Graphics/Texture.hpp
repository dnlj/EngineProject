#pragma once

// OpenGL
#include <glloadgen/gl_core_4_5.hpp>

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Graphics/Image.hpp>
#include <Engine/Graphics/TextureWrap.hpp>
#include <Engine/Graphics/TextureFilter.hpp>
#include <Engine/Graphics/TextureFormat.hpp>


namespace Engine {
	template<int32 D, GLenum Target>
	class Texture {
		private:
			static_assert(1 <= D && D <= 3, "Invalid texture dimensions");
			using Vec = glm::vec<D, glm::i32, glm::defaultp>;
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

			void setStorage(TextureFormat format, Vec size, int mips = 1) {
				if (tex == 0) {
					glCreateTextures(Target, 1, &tex);
				}

				if constexpr (D == 1) {
					// TODO: 1D	
					ENGINE_ERROR("1D Textures are untested.");
					glTextureStorage1D(tex, mips, static_cast<GLenum>(format), size.x);
				} else if constexpr (D == 2) {
					glTextureStorage2D(tex, mips, static_cast<GLenum>(format), size.x, size.y);
				} else if constexpr (D == 3) {
					glTextureStorage3D(tex, mips, static_cast<GLenum>(format), size.x, size.y, size.z);
				}
			}
			
			void setImage(const Image& img) {
				setSubImage(0, img);
			}

			void setSubImage(int32 mip, const Image& img) {
				setSubImage(mip, {}, img);
			}

			void setSubImage(int32 mip, Vec offset, const Image& img) {
				setSubImage(mip, offset, img.size(), img.format(), img.data());
			}

			void setSubImage(int32 mip, Vec offset, Vec size, const Image& img) {
				setSubImage(mip, offset, size, img.format(), img.data());
			}

			void setSubImage(int32 mip, Vec offset, Vec size, PixelFormat format, const void* data) {
				ENGINE_DEBUG_ASSERT(tex != 0, "Attempting to set data of uninitialized texture.");
				// TODO: GL_UNSIGNED_BYTE will need to be a switch or something when we add more pixel formats
				const auto& pixInfo = getPixelFormatInfo(format);

				if constexpr (D == 1) {
					// TODO: 1D	
					ENGINE_ERROR("1D Textures are untested.");
					glTextureSubImage1D(tex, mip, offset.x, size.x,
						pixInfo.glFormat, GL_UNSIGNED_BYTE, data
					);
				} else if constexpr (D == 2) {
					glTextureSubImage2D(tex, mip,
						offset.x, offset.y,
						size.x, size.y,
						pixInfo.glFormat, GL_UNSIGNED_BYTE, data
					);
				} else if constexpr (D == 3) {
					glTextureSubImage3D(tex, mip,
						offset.x, offset.y, offset.z,
						size.x, size.y, size.z,
						pixInfo.glFormat, GL_UNSIGNED_BYTE, data
					);
				}
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
				glTextureParameteri(tex, GL_TEXTURE_WRAP_R, static_cast<GLenum>(wrap));
			}

			void generateMipmaps() {
				ENGINE_DEBUG_ASSERT(tex != 0, "Attempting to generate mipmaps for uninitialized texture.");
				glGenerateTextureMipmap(tex);
			}

			ENGINE_INLINE auto get() const noexcept { return tex; }
	};

	// TODO: untested - using Texture1D = Texture<1, GL_TEXTURE_1D>;
	using Texture2D = Texture<2, GL_TEXTURE_2D>;
	// TODO: untested - using Texture3D = Texture<3, GL_TEXTURE_3D>;
	// TODO: untested - using TextureArray1D = Texture<2, GL_TEXTURE_1D_ARRAY>;
	using TextureArray2D = Texture<3, GL_TEXTURE_2D_ARRAY>;
	// TODO: cubemap
}
