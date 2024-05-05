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
	ENGINE_INLINE inline FT_F26Dot6 addLong(FT_F26Dot6 a, FT_F26Dot6 b) noexcept {
		return FT_F26Dot6(FT_ULong(a) + FT_ULong(b));
	}

	/**
	 * Round an 26.6 number
	 */
	ENGINE_INLINE inline FT_F26Dot6 roundF26d6(FT_F26Dot6 a) noexcept {
		return addLong(a, 32);
	}
}
