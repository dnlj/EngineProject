#pragma once

namespace Engine {
	struct ASCIIColorString {
		constexpr ASCIIColorString(const char* const str) : str{str} {}
		const char* const str;
	};
}
