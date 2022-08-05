#pragma once

// OpenGL
#include <glloadgen/gl_core_4_5.hpp>

// GLM
#include <glm/vec2.hpp>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Gfx/Image.hpp>
#include <Engine/Gfx/TextureWrap.hpp>
#include <Engine/Gfx/TextureFilter.hpp>
#include <Engine/Gfx/TextureFormat.hpp>
#include <Engine/Gfx/TextureHandle.hpp>
#include <Engine/Gfx/TextureType.hpp>


namespace Engine::Gfx {
	class TextureGeneric {
		protected:
			GLuint tex = 0;

		public:
			TextureGeneric() = default;
			TextureGeneric(const TextureGeneric&) = delete;

			ENGINE_INLINE TextureGeneric(TextureGeneric&& other) noexcept {
				*this = std::move(other);
			}

			ENGINE_INLINE TextureGeneric& operator=(TextureGeneric&& other) noexcept {
				swap(*this, other);
				return *this;
			}

			ENGINE_INLINE friend void swap(TextureGeneric& a, TextureGeneric& b) noexcept {
				std::swap(a.tex, b.tex);
			}

			ENGINE_INLINE ~TextureGeneric() {
				glDeleteTextures(1, &tex);
			}

			ENGINE_INLINE operator bool() const noexcept { return tex; }
			ENGINE_INLINE auto get() const noexcept { return tex; }

			void setMinFilter(TextureFilter filter) {
				// TODO: should have a conversion function in Engine::Gfx instead of a lambda here.
				decltype(auto) translate = [](auto f) ENGINE_INLINE -> GLenum {
					switch (f) {
						case TextureFilter::Nearest: {
							return GL_NEAREST;
						}
						case TextureFilter::Bilinear: {
							return GL_LINEAR;
						}
						case TextureFilter::Trilinear: {
							return GL_LINEAR_MIPMAP_LINEAR;
						}
					}
					ENGINE_WARN("Invalid texture minification filter.");
					return GL_NEAREST;
				};
				glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, translate(filter));
			}

			void setMagFilter(TextureFilter filter) {
				// TODO: should have a conversion function in Engine::Gfx instead of a lambda here.
				decltype(auto) translate = [](auto f) ENGINE_INLINE -> GLenum {
					switch (f) {
						case TextureFilter::Nearest: {
							return GL_NEAREST;
						}
						case TextureFilter::Bilinear:
						case TextureFilter::Trilinear: {
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
				// TODO: should probably have a conversion function in Engine::Gfx instead of cast
				glTextureParameteri(tex, GL_TEXTURE_WRAP_S, static_cast<GLenum>(wrap));
				glTextureParameteri(tex, GL_TEXTURE_WRAP_T, static_cast<GLenum>(wrap));
				glTextureParameteri(tex, GL_TEXTURE_WRAP_R, static_cast<GLenum>(wrap));
			}

			void generateMipmaps() {
				ENGINE_DEBUG_ASSERT(tex != 0, "Attempting to generate mipmaps for uninitialized texture.");
				glGenerateTextureMipmap(tex);
			}
	};

	/**
	 * Owns a GPU texture.
	 * @see Image
	 * @see TextureHandle
	 */
	template<int32 D, TextureType Target>
	class Texture : public TextureGeneric {
		private:
			static_assert(1 <= D && D <= 3, "Invalid texture dimensions");
			using Vec = glm::vec<D, glm::i32, glm::defaultp>;

		public:
			using TextureGeneric::TextureGeneric;

			Texture(const Image& img) {
				setAuto(img);
			}

			Texture(const Image& img, TextureFilter filter, TextureWrap wrap) : Texture(img, filter, filter, wrap) {
			}

			Texture(const Image& img, TextureFilter min, TextureFilter mag, TextureWrap wrap) {
				setAuto(img);
				setMinFilter(min);
				setMagFilter(mag);
				setWrap(wrap);
			}

			void setAuto(const Image& img) {
				const auto& format = getPixelFormatInfo(img.format());
				setStorage(format.defaultTexFormat, img.size());
				setImage(img);
			}

			void setStorage(TextureFormat format, Vec size, int mips = 1) {
				glDeleteTextures(1, &tex);
				glCreateTextures(toGL(Target), 1, &tex);

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

			ENGINE_INLINE operator TextureHandle<D, Target>() const noexcept { return TextureHandle<D, Target>{tex}; }
	};

	// TODO: untested - using Texture1D = Texture<1, GL_TEXTURE_1D>;
	using Texture2D = Texture<2, TextureType::Target2D>;
	// TODO: untested - using Texture3D = Texture<3, GL_TEXTURE_3D>;
	// TODO: untested - using Texture1DArray = Texture<2, GL_TEXTURE_1D_ARRAY>;
	using Texture2DArray = Texture<3, TextureType::Target2DArray>;
	// TODO: cubemap
}
