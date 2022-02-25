// FreeType
#include <freetype/freetype.h>

// Engine
#include <Engine/Gui/common.hpp>


namespace Engine::Gui {
	#undef FTERRORS_H_
	#define FT_ERROR_START_LIST
	#define FT_ERROR_END_LIST
	#define FT_ERRORDEF(e, v, s) s,
	const char* getFreeTypeErrorString(const int err) noexcept {
		constexpr const char* errors[] = {
			#include <freetype/fterrors.h>
		};

		if (err < 0 || err >= std::size(errors)) {
			return "Invalid FreeType error code";
		}

		return errors[err];
	}
}
