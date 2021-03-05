#pragma once

// GLM
#include <glm/vec4.hpp>

// OpenGL
#include <glloadgen/gl_core_4_5.hpp>


namespace Engine {
	enum class PixelFormat {
		#define X(Name, Format, Channels, R,G,B,A) Name,
		#include <Engine/Graphics/PixelFormat.xpp>
	};

	struct PixelFormatInfo {
		const GLenum glFormat;
		const int channels;

		union {
			struct {
				const int red;
				const int green;
				const int blue;
				const int alpha;
			};
			const glm::ivec4 bits;
		};
	};

	namespace Detail {
		inline PixelFormatInfo PixelFormat_Info[] = {
			#define X(Name, Format, Channels, R,G,B,A) {Format, Channels, R, G, B, A},
			#include <Engine/Graphics/PixelFormat.xpp>
		};
	}

	ENGINE_INLINE inline constexpr const PixelFormatInfo& getPixelFormatInfo(PixelFormat fmt) noexcept {
		return Detail::PixelFormat_Info[static_cast<int>(fmt)];
	}
}
