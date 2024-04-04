#pragma once

// STD
#include <string>
#include <vector>

// OpenGL
#include <glloadgen/gl_core_4_5.hpp>

// SOIL
#include <soil/SOIL.h>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Gfx/PixelFormat.hpp>


namespace Engine::Gfx {
	/**
	 * Stores image data and format information.
	 * @see Texture
	 */
	class Image {
		private:
			PixelFormat fmt = PixelFormat::NONE;
			glm::ivec2 dims = {};
			std::vector<byte> storage;

		public:
			Image() = default;

			Image(const PixelFormat fmt, const glm::ivec2 dims) : fmt{fmt}, dims{dims} {
				const auto& info = getPixelFormatInfo(fmt);
				storage.resize(dims.x * dims.y * info.channels);
			}

			Image(const std::string& path) : Image{path.c_str()} {
			}

			Image(const char* path) {
				int channels;
				auto image = SOIL_load_image(path, &dims.x, &dims.y, &channels, SOIL_LOAD_RGBA);
				fmt = PixelFormat::RGBA8;

				if (!image) {
					ENGINE_WARN("[SOIL] ", SOIL_last_result(), " - \"", path, "\"");
					fmt = PixelFormat::RGB8;
					dims.x = 8;
					dims.y = 8;
					storage = {
						255, 000, 000,   000, 255, 000,   000, 000, 255,   255, 255, 000,   255, 000, 255,   000, 255, 255,   255, 000, 000,   000, 255, 000,
					    000, 255, 255,   255, 000, 000,   000, 255, 000,   000, 000, 255,   255, 255, 000,   255, 000, 255,   000, 255, 255,   255, 000, 000,
					    255, 000, 255,   000, 255, 255,   255, 000, 000,   000, 255, 000,   000, 000, 255,   255, 255, 000,   255, 000, 255,   000, 255, 255,
					    255, 255, 000,   255, 000, 255,   000, 255, 255,   255, 000, 000,   000, 255, 000,   000, 000, 255,   255, 255, 000,   255, 000, 255,
					    000, 000, 255,   255, 255, 000,   255, 000, 255,   000, 255, 255,   255, 000, 000,   000, 255, 000,   000, 000, 255,   255, 255, 000,
					    000, 255, 000,   000, 000, 255,   255, 255, 000,   255, 000, 255,   000, 255, 255,   255, 000, 000,   000, 255, 000,   000, 000, 255,
					    000, 000, 255,   000, 255, 000,   000, 000, 255,   255, 255, 000,   255, 000, 255,   000, 255, 255,   255, 000, 000,   000, 255, 000,
					    000, 255, 000,   000, 000, 255,   000, 255, 000,   000, 000, 255,   255, 255, 000,   255, 000, 255,   000, 255, 255,   255, 000, 000,
					};
				} else {
					storage.assign(image, image + dims.x * dims.y * channels);
					SOIL_free_image_data(image);
				}
			}

			ENGINE_INLINE operator bool() const noexcept { return fmt != PixelFormat::NONE; }

			ENGINE_INLINE const auto format() const noexcept { return fmt; }
			ENGINE_INLINE const auto& size() const noexcept { return dims; }
			ENGINE_INLINE const byte* data() const noexcept { return storage.data(); }
			ENGINE_INLINE byte* data() noexcept { return storage.data(); }
			ENGINE_INLINE auto sizeBytes() const noexcept { return storage.size(); }

			void fill(glm::u8vec3 rgb) {
				if constexpr (ENGINE_DEBUG) {
					const auto& info = getPixelFormatInfo(fmt);
					ENGINE_DEBUG_ASSERT(info.channels == 3);
					ENGINE_DEBUG_ASSERT(info.bits.r == 8);
					ENGINE_DEBUG_ASSERT(info.bits.g == 8);
					ENGINE_DEBUG_ASSERT(info.bits.b == 8);
					ENGINE_DEBUG_ASSERT(info.bits.a == 0);
				}

				const uint64 sz = storage.size();
				for (uint64 i = 0; i < sz; i += 3) {
					storage[i+0] = rgb.r;
					storage[i+1] = rgb.g;
					storage[i+2] = rgb.b;
				}
			}

			void copySettings(const Image& other) {
				fmt = other.fmt;
				dims = other.dims;
				storage.resize(other.storage.size());
			}

			void flipY() {
				const auto& info = getPixelFormatInfo(fmt);
				const auto rowLength = dims.x * info.channels;
				for (int y = 0; y < dims.y / 2; ++y) {
					std::swap_ranges(
						storage.data() + y * rowLength,
						storage.data() + (y + 1) * rowLength,
						storage.data() + (dims.y - y - 1) * rowLength
					);
				}
			}
	};
}
