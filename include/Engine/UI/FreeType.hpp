#pragma once

// FreeType
#include <freetype/fttypes.h>

/**
 * Helper functions for working with FreeType.
 */
namespace Engine::UI::FreeType {
	/**
	 * Add two 26.6 numbers
	 */
	ENGINE_INLINE inline FT_Long addLong(FT_Long a, FT_Long b) noexcept {
		return FT_Long(FT_ULong(a) + FT_ULong(b));
	}

	/**
	 * Round an 26.6 number
	 */
	ENGINE_INLINE inline FT_Long roundF26d6(FT_Long a) noexcept {
		return addLong(a, 32);
	}
}
