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
#include <Engine/Graphics/PixelFormat.hpp>


namespace Engine {
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
					ENGINE_WARN("[SOIL] ", SOIL_last_result());
					fmt = PixelFormat::RGB8;
					dims.x = 2;
					dims.y = 2;
					storage = {
						255, 000, 000,    000, 255, 000,
						000, 255, 000,    255, 000, 000,
					};
				} else {
					storage.assign(image, image + dims.x * dims.y * channels);
					SOIL_free_image_data(image);
				}

				ENGINE_LOG("Img: ", path, " ", dims.x, " ", dims.y);
			}

			ENGINE_INLINE operator bool() const noexcept { return fmt != PixelFormat::NONE; }

			ENGINE_INLINE const auto format() const noexcept { return fmt; }
			ENGINE_INLINE const auto& size() const noexcept { return dims; }
			ENGINE_INLINE const byte* data() const noexcept { return storage.data(); }
			ENGINE_INLINE byte* data() noexcept { return storage.data(); }

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
