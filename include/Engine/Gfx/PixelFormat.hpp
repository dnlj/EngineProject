#pragma once

// GLM
#include <glm/vec4.hpp>

// OpenGL
#include <glloadgen/gl_core_4_5.hpp>
#include <Engine/Gfx/TextureFormat.hpp>


namespace Engine::Gfx {
	enum class PixelFormat {
		#define X(Name, Format, Tex, Channels, R,G,B,A) Name,
		#include <Engine/Gfx/PixelFormat.xpp>
	};

	struct PixelFormatInfo {
		GLenum glFormat;
		TextureFormat defaultTexFormat;
		int channels;

		union {
			struct {
				int red;
				int green;
				int blue;
				int alpha;
			};
			glm::ivec4 bits;
		};
	};

	namespace Detail {
		inline constexpr PixelFormatInfo PixelFormat_Info[] = {
			#define X(Name, Format, Tex, Channels, R,G,B,A) {Format, TextureFormat::Tex, Channels, R, G, B, A},
			#include <Engine/Gfx/PixelFormat.xpp>
		};
	}

	ENGINE_INLINE inline constexpr const PixelFormatInfo& getPixelFormatInfo(PixelFormat fmt) noexcept {
		return Detail::PixelFormat_Info[static_cast<int>(fmt)];
	}
}
